// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>

//============================================================================
// Name        : uds.c
// Author      : Dorothea Ehrl
// Version     : 0.1
// Copyright   : MIT
// Description : Handles UDS messages from CAN bus
//============================================================================

#include <string.h>
#include <stdio.h>

#include "uds.h"

#define REQUEST                                                 0
#define RESPONSE                                                1


// TODO should I add these UDS_Msg functions to the header?
struct UDS_Msg {
    uint32 len;
    uint8 data[]; // flexible array member
};
typedef struct UDS_Msg UDS_Msg;

uint8 getSID(UDS_Msg *msg);
void getData(UDS_Msg *msg, uint8 *data, uint32 *len);
uint32 getDataLength(UDS_Msg *msg);

uint8 getSID(UDS_Msg *msg){
    if (msg->len == 0){
        return 0;
    }
    uint8 first_byte = msg->data[0];
    return (first_byte & 0x3f); // TODO 6 Bit of first byte are SID, right?
}

uint16 getDID(UDS_Msg *msg){
    if (msg->len == 0){
        return 0;
    }
    uint8 high = msg->data[1];                              // DID high Byte
    uint8 low = msg->data[2];                               // DID low Byte
    uint16 did = ((uint16) high) << 8;
    did |= low;
    return did;
}

void getData(UDS_Msg *msg, uint8 *data, uint32 *len){
    data = msg->data;
}

uint32 getDataLength(UDS_Msg *msg){
    return msg->len;
}

// TODO remove debug print
void debug_print(uint8 *data, uint32 len){
    FILE * f3 = fopen("terminal window 3", "rw");
    fprintf(f3, "\nMessage:\n");
    for(int i = 0; i < len; i++){
        fprintf(f3, "[%x]", data[i]);
    }
    fprintf(f3, "\n");
    fclose(f3);
}

static void diagnosticSessionControl(void){
    int response_len;
    uint8 session = getSession();
    uint8* response_msg = _create_diagnostic_session_control(&response_len, RESPONSE, session);
    // TODO send response_msg
    debug_print(response_msg, response_len);
    free(response_msg);
}

static void readDataByIdentifier(uint16 did){
	uint8 name[] = "AMOS FBL 24"; // TODO which name? And global var etc.?
	uint8 *data;
    int data_len = 0;
    switch(did)
    {
        case FBL_DID_SYSTEM_NAME:
            data = name;
            data_len = sizeof(name);
            break;
        case FBL_DID_PROGRAMMING_DATE:
            break;
        case FBL_DID_BL_KEY_ADDRESS:
            break;
        case FBL_DID_BL_KEY_GOOD_VALUE:
            break;
        case FBL_DID_CAN_BASE_MASK:
            break;
        case FBL_DID_CAN_ID:
            break;
        case FBL_DID_BL_WRITE_START_ADD_CORE0:
            break;
        case FBL_DID_BL_WRITE_END_ADD_CORE0:
            break;
        case FBL_DID_BL_WRITE_START_ADD_CORE1:
            break;
        case FBL_DID_BL_WRITE_END_ADD_CORE1:
            break;
        case FBL_DID_BL_WRITE_START_ADD_CORE2:
            break;
        case FBL_DID_BL_WRITE_END_ADD_CORE2:
            break;
        default:
            ;
            // TODO send error message
    }
    int response_len;
	uint8* response_msg = _create_read_data_by_ident(&response_len, 1, did, data, data_len);
    // TODO send response_msg
    debug_print(response_msg, response_len);
    free(response_msg);
}

//void ecuReset();
//void securityAccess();
//void testerPresent();

//void readMemoryByAddress(address);
//void writeDataByIdentifier(data);
//void requestDownload();
//void requestUpload();
//void transferData(data);
//void requestTransferExit();
//void negativeResponse(data);

void handleRXUDS(uint8* data, uint32 data_len){
    uint8 array[data_len + sizeof(uint32)]; // TODO change if incoming data format is different

    UDS_Msg* msg = (UDS_Msg*) array;
    msg->len = data_len;
    memcpy(msg->data, data, data_len);
    debug_print(msg->data, msg->len);

    uint16 did; // only needed for data by identifier, but cannot be declared inside switch statement

    // parse incoming data by SID and call function for SID
    uint8 SID = getSID(msg);
    switch (SID)
    {
        case FBL_DIAGNOSTIC_SESSION_CONTROL:
            diagnosticSessionControl();
            break;
        case FBL_ECU_RESET:
            break;
        case FBL_SECURITY_ACCESS:
            break;
        case FBL_TESTER_PRESENT:
            break;
        case FBL_READ_DATA_BY_IDENTIFIER:
            did = getDID(msg);
            readDataByIdentifier(did);
            break;
        case FBL_READ_MEMORY_BY_ADDRESS:
            break;
        case FBL_WRITE_DATA_BY_IDENTIFIER:
            break;
        case FBL_REQUEST_DOWNLOAD:
            break;
        case FBL_REQUEST_UPLOAD:
            break;
        case FBL_TRANSFER_DATA:
            break;
        case FBL_REQUEST_TRANSFER_EXIT:
            break;
        default:
            // TODO send error tx
            ;
    }

}

