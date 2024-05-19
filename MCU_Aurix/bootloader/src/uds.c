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

#define REQUEST                         0
#define RESPONSE                        1


// TODO should I add these UDS_Msg functions to the header?
struct UDS_Msg {
    uint32 len;
    uint8 data[]; // flexible array member
};

typedef struct UDS_Msg UDS_Msg;

uint8 getSID(UDS_Msg *msg);
void getData(UDS_Msg *msg, uint8 *data, uint32 *len);
//uint32 getID(UDS_Msg *msg);
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

}

void diagnosticSessionControl(UDS_Msg *msg){

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

