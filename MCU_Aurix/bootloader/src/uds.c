// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>
// SPDX-FileCopyrightText: 2024 Sebastian Rodriguez <r99@melao.de>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : uds.c
// Author      : Dorothea Ehrl, Sebastian Rodriguez, Michael Bauer
// Version     : 0.2
// Copyright   : MIT
// Description : UDS Layer implementation
//============================================================================

#include <string.h>
#include <stdio.h>

#include "uds.h"
#include "isotp.h"
#include "session_manager.h"
#include "memory.h"
#include "flashing.h"

#define REQUEST                                                 0
#define RESPONSE                                                1

static isoTP* iso;

//============================================================================
// Private Helper Function declarations
//============================================================================

static uint8_t getSID(UDS_Msg *msg);
static uint16_t getDID(UDS_Msg *msg);
static uint32_t getMemoryAddress(UDS_Msg *msg);
static uint16_t getNoBytes(UDS_Msg *msg);

//============================================================================
// Init + Close
//============================================================================


void uds_init(void){
    iso = isotp_init();
}

void uds_close(void){
    close_isoTP(iso);
}

//============================================================================
// RX
//============================================================================

void uds_handleRX(uint8_t* data, uint32_t data_len){
    uint8_t array[sizeof(uint32_t) + data_len ]; // TODO change if incoming data format is different

    UDS_Msg* msg = (UDS_Msg*) array;
    msg->len = data_len;
    memcpy(msg->data, data, data_len);

    // Flag for session control
    boolean responded = 1;

    uint16_t did; // only needed for data by identifier, but cannot be declared inside switch statement

    // parse incoming data by SID and call function for SID
    uint8_t SID = getSID(msg);

    // Directly check if SID is allowed in current session, 0 = is allowed
    uint8_t nrc = SIDallowedInCurrentSession(SID);
    if(nrc){
        uds_neg_response(SID, nrc);
        return;
    }

    // Check on the different SIDs
    switch (SID)
    {
        case FBL_DIAGNOSTIC_SESSION_CONTROL:
            if(msg->len != 2){
                uds_neg_response(SID, FBL_RC_INCORRECT_MSG_LEN_OR_INV_FORMAT);
                return;
            }

            uds_diagnostic_session_control(msg->data[1]);
            break;

        case FBL_ECU_RESET:
            if(msg->len != 2){
                uds_neg_response(SID, FBL_RC_INCORRECT_MSG_LEN_OR_INV_FORMAT);
                return;
            }

            uds_ecu_reset(msg->data[1]);
            break;

        case FBL_SECURITY_ACCESS:
            if(msg->len < 2){ // TODO: Need to be adjusted based on key
                uds_neg_response(SID, FBL_RC_INCORRECT_MSG_LEN_OR_INV_FORMAT);
                return;
            }

            if(msg->data[1] == FBL_SEC_ACCESS_SEED){
                uint8_t seed[SEED_LENGTH];
                generateSeed(seed); //TODO implement getKey or is it Authenticate()?
                uds_security_access(msg->data[1], seed, SEED_LENGTH);
            }
            else if (msg->data[1] == FBL_SEC_ACCESS_VERIFY_KEY)
            {
                uint8_t access_granted = verifyKey(msg->data + 2, msg->len - 2); //TODO implement verifyKey
                if (access_granted)
                {
                    uds_security_access(msg->data[1], NULL, 0);
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
            if (msg->data[1] == FBL_TESTER_PRES_WITH_RESPONSE){
                uds_tester_present();
            }
            else if (msg->data[1] == FBL_TESTER_PRES_WITHOUT_RESPONSE){
                // Just ignore the tester present
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
            responded = 0;
            uds_neg_response(SID, FBL_RC_SERVICE_NOT_SUPPORTED);
    }

    if(responded){
        // Call session control to indicate that valid communication was received
        sessionControl();
    }
}

//============================================================================
// TX
//============================================================================


//============================================================================
// Diagnostic and Communication Management

/**
 * Method to set the session and sending the response for diagnostic session control
 */
void uds_diagnostic_session_control(uint8_t session){
    tx_reset_isotp_buffer(iso);
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;

    // Set Session if possible
    uint8_t nrc = setSession(session);
    if(nrc)
        uds_neg_response(FBL_DIAGNOSTIC_SESSION_CONTROL, nrc);

    // Create Response for session change
    int len;
    uint8_t *msg = _create_diagnostic_session_control(&len, RESPONSE, getSession());
    isotp_send(iso, msg, len);
    free(msg);
}

/**
 * Method to send the response for ecu reset
 */
void uds_ecu_reset(uint8_t reset_type){
    tx_reset_isotp_buffer(iso);
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;

    uint8_t nrc = isResetTypeAvailable(reset_type);
    if (nrc){
        uds_neg_response(FBL_ECU_RESET, nrc);
        return;
    }

    // Create Response for reset
    int len;
    uint8_t *msg = _create_ecu_reset(&len, RESPONSE, reset_type);
    isotp_send(iso, msg, len);
    free(msg);

    // Trigger the reset
    resetECU(reset_type);
}

/**
 * Method to send the response for security access
 */
void uds_security_access(uint8_t request_type, uint8_t *key, uint8_t key_len){
    tx_reset_isotp_buffer(iso);
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;
    int len;
    uint8_t *msg = _create_security_access(&len, RESPONSE, request_type, key, key_len);
    isotp_send(iso, msg, len);
    free(msg);
}

/**
 * Method to send the response for tester present
 */
void uds_tester_present(void){
    tx_reset_isotp_buffer(iso);
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;
    int len;
    uint8_t *msg = _create_tester_present(&len, RESPONSE, FBL_TESTER_PRES_WITH_RESPONSE);
    isotp_send(iso, msg, len);
    free(msg);
}

//============================================================================
// Data Transmission

/**
 * Method to send the response for read data by identifier
 */
void uds_read_data_by_identifier(uint16_t did){
    uint8_t len = 0;
    uint8_t nrc = 0;

    uint8_t* data = readData(did, &len, &nrc);
    if(nrc){
        uds_neg_response(FBL_READ_DATA_BY_IDENTIFIER, nrc);
        return;
    }

    tx_reset_isotp_buffer(iso);
    int response_len;
    uint8_t* response_msg = _create_read_data_by_ident(&response_len, RESPONSE, did, data, len);
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;
    isotp_send(iso, response_msg, response_len);
    free(response_msg);
    free(data);
}

/**
 * Method to send the response for read memory by address
 */
void uds_read_memory_by_address(uint32_t address, uint16_t noBytesToRead){
    tx_reset_isotp_buffer(iso);
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
}

/**
 * Method to send the response for write data by identifier
 */
void uds_write_data_by_identifier(uint16_t did, uint8_t* data, uint8_t data_len){
    uint8_t nrc = writeData(did, data, data_len);
    if(nrc){
        uds_neg_response(FBL_WRITE_DATA_BY_IDENTIFIER, nrc);
        return;
    }
    tx_reset_isotp_buffer(iso);
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;
    int len;
    uint8_t *msg = _create_write_data_by_ident(&len, RESPONSE, did, 0, 0);
    isotp_send(iso, msg, len);
    free(msg);
}

//============================================================================
// Upload | Download

/**
 * Method to send the response for request download
 */
void uds_request_download(void){

    // TODO needs to be implemented
}

/**
 * Method to send the response for request upload
 */
void uds_request_upload(void){
    // TODO needs to be implemented
}

/**
 * Method to process the transfer data, no response is provided
 */
void uds_transfer_data(uint8_t* data){
    // TODO needs to be implemented
}

/**
 * Method to send the response for transfer exit
 */
void uds_request_transfer_exit(void){
    // TODO needs to be implemented
}

//============================================================================
// Negative Response - Common Response Codes

void uds_neg_response(uint8_t reg_sid ,uint8_t neg_code){
    tx_reset_isotp_buffer(iso);
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;
    int len;
    uint8_t *msg = _create_neg_response(&len, reg_sid, neg_code);
    isotp_send(iso, msg, len);
    free(msg);
}

//============================================================================
// Private Helper Functions
//============================================================================

static uint8_t getSID(UDS_Msg *msg){
    if (msg->len == 0){
        return 0;
    }
    return msg->data[0];
}

static uint16_t getDID(UDS_Msg *msg){
    if (msg->len == 0){
        return 0;
    }

    uint16_t did = 0;
    uint8_t max_idx = 2;
    for(int i = max_idx; i >= 1; i--){
        did |= (msg->data[i] << (8*(max_idx-i)));
    }
    return did;
}

static uint32_t getMemoryAddress(UDS_Msg *msg){
    if (msg->len == 0){
        return 0;
    }
    uint32_t addr = 0;
    uint8_t max_idx = 4;
    for(int i = 4; i >= 1; i--){
        addr |= (msg->data[i] << (8*(max_idx-1)));
    }
    return addr;
}

static uint16_t getNoBytes(UDS_Msg *msg){
    if (msg->len == 0){
        return 0;
    }
    uint16_t bytes = 0;
    for(int i = 6; i >= 5; i--){
        bytes |= (msg->data[i] << (8*(i-5)));
    }
    return bytes;
}
