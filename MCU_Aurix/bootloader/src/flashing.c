// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

//============================================================================
// Name        : flashing.c
// Author      : Dorothea Ehrl, Michael Bauer, Wiktor Pilarczyk
// Version     : 0.1
// Copyright   : MIT
// Description : Manages flash data
//============================================================================

#include "flashing.h"
#include "uds_comm_spec.h"
#include "memory.h"
#include "flash_driver.h"

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

static inline bool addrInCoreRangeCheck(uint32_t addr, uint32_t data_len, uint16_t start_addr_ID, uint16_t end_addr_ID) {
    uint32_t core_start_add = flashingGetDIDData(start_addr_ID);
    uint32_t core_end_add = flashingGetDIDData(end_addr_ID);
    if((core_start_add > 0 && core_end_add > 0) && (addr >= core_start_add && addr < core_end_add)){
        // address belongs to core
        if(addr + data_len <= core_end_add)
            return true;
    }
    return false;
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
    if(!addrInCoreRangeCheck(address, data_len, FBL_DID_BL_WRITE_START_ADD_CORE0, FBL_DID_BL_WRITE_END_ADD_CORE0) &&
       !addrInCoreRangeCheck(address, data_len, FBL_DID_BL_WRITE_START_ADD_CORE1, FBL_DID_BL_WRITE_END_ADD_CORE1) &&
       !addrInCoreRangeCheck(address, data_len, FBL_DID_BL_WRITE_START_ADD_CORE2, FBL_DID_BL_WRITE_END_ADD_CORE2))
    {
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

    // TODO: Erase Flash Necessary? See flashWrite internal

    // TODO: Check Edge cases -> Decide what to do when data_len % size(uint32_t) > 0!
    bool flashed = flashWrite(address, (uint32_t*)(&data), data_len / sizeof(uint32_t));

    if(!flashed)
        return FBL_RC_FAILURE_PREVENTS_EXEC_OF_REQUESTED_ACTION;

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
