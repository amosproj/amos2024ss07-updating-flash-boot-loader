// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>, Sebastian Rodriguez <r99@melao.de>

//============================================================================
// Name        : uds.c
// Author      : Dorothea Ehrl, Sebastian Rodriguez
// Version     : 0.1
// Copyright   : MIT
// Description : UDS Layer implementation
//============================================================================

//TODO: change isoTP handling and discuss with Leon

#include <string.h>
#include <stdio.h>

#include "uds.h"
#include "isotp.h"
#include "session_manager.h"
#include "memory.h"
#include "flashing.h"

#define REQUEST                                                 0
#define RESPONSE                                                1


// TODO should I add these UDS_Msg functions to the header?
struct UDS_Msg {
    uint32_t len;
    uint8_t data[]; // flexible array member
};
typedef struct UDS_Msg UDS_Msg;

uint8_t getSID(UDS_Msg *msg){
    if (msg->len == 0){
        return 0;
    }
    uint8_t first_byte = msg->data[0];
    return (first_byte & 0x3f); // TODO 6 Bit of first byte are SID, right?
}

uint16_t getDID(UDS_Msg *msg){
    if (msg->len == 0){
        return 0;
    }
    uint8_t high = msg->data[1];                              // DID high Byte
    uint8_t low = msg->data[2];                               // DID low Byte
    uint16_t did = ((uint16_t) high) << 8;
    did |= low;
    return did;
}

uint32_t getMemoryAddress(UDS_Msg *msg){
    if (msg->len == 0){
        return 0;
    }
    uint32_t addr = 0;
    addr |= (msg->data[1] << 24);
    addr |= (msg->data[2] << 16);
    addr |= (msg->data[3] << 8);
    addr |= msg->data[4];

    return addr;
}

uint16_t getNoBytes(UDS_Msg *msg){
    if (msg->len == 0){
        return 0;
    }
    uint16_t bytes = 0;
    bytes |= (msg->data[5] << 8);
    bytes |= msg->data[6];

    return bytes;
}

uint8_t uds_session_access(uint8_t sid){
    uint8_t session = getSession();
    if (session == FBL_DIAG_SESSION_DEFAULT){
        uds_neg_response(sid, FBL_RC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION);
        return 0;
    }
    else if (session == FBL_DIAG_SESSION_PROGRAMMING)
    {
        if (!isAuthorized())
        {
            uds_neg_response(sid, FBL_RC_SECURITY_ACCESS_DENIED);
            return 0;
        }
    }
    return 1;
}

void uds_diagnostic_session_control(void){
    isoTP* iso = isotp_init();
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;
    int len;
    uint8_t session = getSession();
    uint8_t *msg = _create_diagnostic_session_control(&len, RESPONSE, session);
    isotp_send(iso, msg, len);
    free(msg);
    close_isoTP(iso);
}

void uds_ecu_reset(uint8_t reset_type){
    isoTP* iso = isotp_init();
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;
    uint32_t len;
    uint8_t *msg = _create_ecu_reset(&len, 1, reset_type);
    isotp_send(iso, msg, len);
    free(msg);

    close_isoTP(iso);
}

void uds_read_data_by_identifier(uint16_t did){
    uint8_t* data;
    uint8_t* data_len;
    if(readData(did, data, data_len)){
        uds_neg_response(FBL_READ_DATA_BY_IDENTIFIER, FBL_NEGATIVE_RESPONSE);
        return;
    }
    // TODO data initialized because readData is not implemented yet
    data = "AMOS FBL 24";
    *data_len = sizeof("AMOS FBL 24");

    int response_len;
    uint8_t* response_msg = _create_read_data_by_ident(&response_len, RESPONSE, did, data, *data_len);
    isoTP* iso = isotp_init();
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;
    isotp_send(iso, response_msg, response_len);
    free(response_msg);
    close_isoTP(iso);
    free(data);
}

void uds_security_access(uint8_t request_type, uint8_t *key, uint8_t key_len){
    isoTP* iso = isotp_init();
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;
    int len;
    uint8_t *msg = _create_security_access(&len, RESPONSE, request_type, key, key_len);
    isotp_send(iso, msg, len);
    free(msg);
    close_isoTP(iso);
}

void uds_neg_response(uint8_t reg_sid ,uint8_t neg_code){
    isoTP* iso = isotp_init();
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;
    int len;

    uint8_t *msg = _create_neg_response(&len, reg_sid, neg_code);
    isotp_send(iso, msg, len);
    free(msg);
    close_isoTP(iso);
}

void uds_tester_present(void){
    isoTP* iso = isotp_init();
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;
    int len;
    uint8_t *msg = _create_tester_present(&len, RESPONSE, FBL_TESTER_PRES_WITH_RESPONSE);
    isotp_send(iso, msg, len);
    free(msg);
    close_isoTP(iso);
}

void uds_read_memory_by_address(uint32_t address, uint16_t noBytesToRead){
    if(!uds_session_access(FBL_READ_MEMORY_BY_ADDRESS)){
        return;
    }
    isoTP* iso = isotp_init();
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;
    int len;

    uint8_t data[noBytesToRead];
    if(readMemory(address, noBytesToRead, data)){
        uds_neg_response(FBL_READ_MEMORY_BY_ADDRESS, FBL_NEGATIVE_RESPONSE);
        return;
    }
    uint8_t *msg = _create_read_memory_by_address(&len, RESPONSE, address, 0, data, noBytesToRead);
    isotp_send(iso, msg, len);
    free(msg);
    close_isoTP(iso);
}

void uds_write_data_by_identifier(uint16_t did, uint8_t* data, uint8_t data_len){
    if(!uds_session_access(FBL_WRITE_DATA_BY_IDENTIFIER)){
        return;
    }
    if(!writeData(did, data, data_len)){
        uds_neg_response(FBL_WRITE_DATA_BY_IDENTIFIER, FBL_NEGATIVE_RESPONSE);
        return;
    }
    isoTP* iso = isotp_init();
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;
    int len;
    uint8_t *msg = _create_write_data_by_ident(&len, RESPONSE, did, 0, 0);
    isotp_send(iso, msg, len);
    free(msg);
    close_isoTP(iso);
}

void uds_request_download(void){
    if(!uds_session_access(FBL_REQUEST_DOWNLOAD)){
        return;
    }
    // TODO needs to be implemented
}

void uds_request_upload(void){
    if(!uds_session_access(FBL_REQUEST_UPLOAD)){
        return;
    }
    // TODO needs to be implemented
}

void uds_transfer_data(uint8_t* data){
    // TODO needs to be implemented
}

void uds_request_transfer_exit(void){
    // TODO needs to be implemented
}

void uds_handleRX(uint8_t* data, uint32_t data_len){
    uint8_t array[data_len + sizeof(uint32_t)]; // TODO change if incoming data format is different

    UDS_Msg* msg = (UDS_Msg*) array;
    msg->len = data_len;
    memcpy(msg->data, data, data_len);

    uint16_t did; // only needed for data by identifier, but cannot be declared inside switch statement
    uint8_t reset_type;
    uint8_t response_type;

    // parse incoming data by SID and call function for SID
    uint8_t SID = getSID(msg);
    switch (SID)
    {
        case FBL_DIAGNOSTIC_SESSION_CONTROL:
            uds_diagnostic_session_control();
            break;

        case FBL_ECU_RESET:
            reset_type = msg->data[1];
            if (reset_type == FBL_ECU_RESET_POWERON || reset_type == FBL_ECU_RESET_COLD_POWERON || reset_type == FBL_ECU_RESET_WARM_POWERON){
                uds_ecu_reset(reset_type);
                resetECU();
            }
            else
            {
                uds_neg_response(FBL_ECU_RESET, FBL_RC_INVALID_KEY);
            }
            break;

        case FBL_SECURITY_ACCESS:
            reset_type = msg->data[1];
            if(reset_type == FBL_SEC_ACCESS_SEED){
                uint8_t seed[SEED_LENGTH];
                generateSeed(seed); //TODO implement getKey or is it Authenticate()?
                uds_security_access(reset_type, seed, SEED_LENGTH);
            }
            else if (reset_type == FBL_SEC_ACCESS_VERIFY_KEY)
            {   
                uint8_t access_granted = verifyKey(msg->data + 2, msg->len - 2); //TODO implement verifyKey
                if (access_granted)
                {
                    uds_security_access(reset_type, NULL, 0);
                }
                else
                {
                    uds_neg_response(FBL_SECURITY_ACCESS, FBL_RC_INVALID_KEY);
                }
            }
            else
            {
                uds_neg_response(FBL_SECURITY_ACCESS, FBL_NEGATIVE_RESPONSE);
            }
            break;

        case FBL_TESTER_PRESENT:
            response_type = msg->data[1];
            if (response_type == FBL_TESTER_PRES_WITH_RESPONSE){
                uds_tester_present();
            }
            else if (response_type == FBL_TESTER_PRES_WITHOUT_RESPONSE){
                //TODO do nothing?
            }
            else
            {
                uds_neg_response(FBL_TESTER_PRESENT, FBL_NEGATIVE_RESPONSE);
            }
            
            break;
        case FBL_READ_DATA_BY_IDENTIFIER:
            did = getDID(msg);
            uds_read_data_by_identifier(did);
            break;
        case FBL_READ_MEMORY_BY_ADDRESS:
            uds_read_memory_by_address(getMemoryAddress(msg), getNoBytes(msg));
            break;
        case FBL_WRITE_DATA_BY_IDENTIFIER:
            did = getDID(msg);
            uds_write_data_by_identifier(did, msg->data + 3, msg->len - 3);
            break;
        case FBL_REQUEST_DOWNLOAD:
            uds_request_download();
            break;
        case FBL_REQUEST_UPLOAD:
            uds_request_upload();
            break;
        case FBL_TRANSFER_DATA:
            uds_transfer_data(data);
            break;
        case FBL_REQUEST_TRANSFER_EXIT:
            uds_request_transfer_exit();
            break;
        default:
            uds_neg_response(SID, FBL_NEGATIVE_RESPONSE);
    }
}

