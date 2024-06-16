// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>


//============================================================================
// Name        : flashing.c
// Author      : Dorothea Ehrl, Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Manages flash data
//============================================================================

#include "flashing.h"
#include "uds_comm_spec.h"
#include "memory.h"

enum FLASHING_STATE {DOWNLOAD, UPLOAD, TRANSFER_DATA, IDLE};

typedef struct {
    uint32_t buffer;
    uint32_t startAddr;
    uint32_t endAddr;
    enum FLASHING_STATE state;
} Flashing_Internal;

Flashing_Internal flashing_int_data;

//============================================================================
// Internal helper function
//============================================================================

uint32_t flashingGetDIDData(uint16_t DID){
    uint32_t data = 0;

    uint8_t len, nrc;
    uint8_t *read = readData(DID, &len, &nrc);
    if(!nrc){
        if(len == 4){ // uint32_t
            data |= (read[0] << 24);
            data |= (read[1] << 16);
            data |= (read[2] << 8);
            data |= read[3];
        }
    }
    return data;
}

//============================================================================
// Public
//============================================================================
void flashingInit(void){
    flashing_int_data.buffer = 0;
    flashing_int_data.startAddr = 0;
    flashing_int_data.endAddr = 0;
    flashing_int_data.state = IDLE;
}

uint8_t flashingRequestDownload(uint32_t address, uint32_t data_len){

    if (flashing_int_data.state != IDLE)
        return FBL_RC_UPLOAD_DOWNLOAD_NOT_ACCEPTED;

    // Check on Flash Memory to accept download
    uint32_t core0_start_add = flashingGetDIDData(FBL_DID_BL_WRITE_START_ADD_CORE0);
    uint32_t core0_end_add = flashingGetDIDData(FBL_DID_BL_WRITE_END_ADD_CORE0);
    if((core0_start_add > 0 && core0_end_add > 0) && (address >= core0_start_add && address < core0_end_add)){
        // address belongs to core 0
        if(address + data_len > core0_end_add)
            return FBL_RC_REQUEST_OUT_OF_RANGE;

    }

    uint32_t core1_start_add = flashingGetDIDData(FBL_DID_BL_WRITE_START_ADD_CORE1);
    uint32_t core1_end_add = flashingGetDIDData(FBL_DID_BL_WRITE_END_ADD_CORE1);
    if((core1_start_add > 0 && core1_end_add > 0) && (address >= core1_start_add && address < core1_end_add)){
        // address belongs to core 1
        if(address + data_len > core1_end_add)
            return FBL_RC_REQUEST_OUT_OF_RANGE;

    }

    uint32_t core2_start_add = flashingGetDIDData(FBL_DID_BL_WRITE_START_ADD_CORE2);
    uint32_t core2_end_add = flashingGetDIDData(FBL_DID_BL_WRITE_END_ADD_CORE2);
    if((core2_start_add > 0 && core2_end_add > 0) && (address >= core2_start_add && address < core2_end_add)){
        // address belongs to core 2
        if(address + data_len > core2_end_add)
            return FBL_RC_REQUEST_OUT_OF_RANGE;
    }

    // Store base address for flashing
    flashing_int_data.startAddr = address;
    flashing_int_data.endAddr = address + data_len;

    // Identify the max package size
    flashing_int_data.buffer = MAX_ISOTP_MESSAGE_LEN - 8; // Excluding: SID + address (5 bytes), keep some buffer

    // Setup Flashing Mode
    flashing_int_data.state = TRANSFER_DATA;

    return 0;
}

uint8_t flashingRequestUpload(uint32_t address, uint32_t data_len){
    return FBL_RC_SERVICE_NOT_SUPPORTED;
}

uint8_t flashingTransferData(uint32_t address, uint8_t* data, uint32_t data_len){
    if (flashing_int_data.state != TRANSFER_DATA || flashing_int_data.startAddr == 0 || flashing_int_data.endAddr == 0)
        return FBL_RC_REQUEST_SEQUENCE_ERROR;

    if(address < flashing_int_data.startAddr || address > flashing_int_data.endAddr){
        return FBL_RC_REQUEST_OUT_OF_RANGE;
    }

    // TODO: Erase Flash
    // TODO: Store the data to memory
    return 0;
}

uint8_t flashingTransferExit(uint32_t address){
    if(address != 0 && flashing_int_data.startAddr != address)
        return FBL_RC_REQUEST_OUT_OF_RANGE;

    flashing_int_data.startAddr = 0;
    flashing_int_data.endAddr = 0;
    flashing_int_data.buffer = 0;

    flashing_int_data.state = IDLE;
    return 0;
}

uint32_t flashingGetFlashBufferSize(){
    return flashing_int_data.buffer;
}
