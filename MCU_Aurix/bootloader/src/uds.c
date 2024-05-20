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

#include "uds.h"
#include "uds_comm_spec.h"

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

void getData(UDS_Msg *msg, uint8 *data, uint32 *len){
    data = msg->data;
}

uint32 getDataLength(UDS_Msg *msg){
    return msg->len;
}

void handleRXUDS(uint8* data, uint32 data_len){
    uint8 array[data_len + sizeof(uint32)]; // TODO change if incoming data format is different

    UDS_Msg* msg = (UDS_Msg*) array;
    msg->len = data_len;
    memcpy(data, msg->data, data_len);

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

void diagnosticSessionControl(){

    int response_len;
    uint8 session = FBL_DIAG_SESSION_DEFAULT;//getSession(); // TODO getSession not really implemented yet!
    uint8* response_msg = _create_diagnostic_session_control(&response_len, RESPONSE, session);
    // TODO send
}

//void ecuReset();
//void securityAccess();
//void testerPresent();
//void readDataByIdentifier(identifier);
//void readMemoryByAddress(address);
//void writeDataByIdentifier(data);
//void requestDownload();
//void requestUpload();
//void transferData(data);
//void requestTransferExit();
//void negativeResponse(data);

