// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>
// SPDX-FileCopyrightText: 2024 Sebastian Rodriguez <r99@melao.de>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@faud.de>

//============================================================================
// Name        : memory.c
// Author      : Dorothea Ehrl, Sebastian Rodriguez, Michael Bauer
// Version     : 0.3
// Copyright   : MIT
// Description : Manages writing and returning data in memory
//============================================================================

#include <string.h>
#include <stdlib.h>     /* calloc, exit, free */

#include "memory.h"
#include "uds_comm_spec.h"
#include "flash_driver.h"

typedef struct {
        uint8_t did_system_name[FBL_DID_SYSTEM_NAME_BYTES_SIZE];
        uint8_t did_programming_date[FBL_DID_PROGRAMMING_DATE_BYTES_SIZE];
        uint8_t did_bl_key_address[FBL_DID_BL_KEY_ADDRESS_BYTES_SIZE];
        uint8_t did_bl_key_good_value[FBL_DID_BL_KEY_GOOD_VALUE_BYTES_SIZE];
        uint8_t did_can_base_mask[FBL_DID_CAN_BASE_MASK_BYTES_SIZE];
        uint8_t did_can_id[FBL_DID_CAN_ID_BYTES_SIZE];
        uint8_t did_bl_write_start_add_core0[FBL_DID_BL_WRITE_START_ADD_CORE0_BYTES_SIZE];
        uint8_t did_bl_write_end_add_core0[FBL_DID_BL_WRITE_END_ADD_CORE0_BYTES_SIZE];
        uint8_t did_bl_write_start_add_core1[FBL_DID_BL_WRITE_START_ADD_CORE1_BYTES_SIZE];
        uint8_t did_bl_write_end_add_core1[FBL_DID_BL_WRITE_END_ADD_CORE1_BYTES_SIZE];
        uint8_t did_bl_write_start_add_core2[FBL_DID_BL_WRITE_START_ADD_CORE2_BYTES_SIZE];
        uint8_t did_bl_write_end_add_core2[FBL_DID_BL_WRITE_END_ADD_CORE2_BYTES_SIZE];
} Memory_Data;

boolean init = FALSE;
Memory_Data memData;

//============================================================================
// Internal helper function
//============================================================================
static inline void write_to_variable(uint8_t len, uint8_t *data, uint8_t* var){
    for(int i = 0; i < len; i++){
        var[i] = data[i];
    }
}

static inline uint8_t *prepare_system_name_message(uint8_t *len, uint8_t *data){
    uint8_t* ret_data = (uint8_t*)calloc(*len, sizeof(uint8_t));
    for(int i = 0; i < *len; i++){
        ret_data[i] = data[i];
        if(ret_data[i] == 0x00){ // End of String
            *len = i+1;
            break;
        }
    }

    return ret_data;
}

static inline uint8_t *prepare_message(uint8_t *len, uint8_t *data){
    uint8_t* ret_data = (uint8_t*)calloc(*len, sizeof(uint8_t));
    for(int i = 0; i < *len; i++){
        ret_data[i] = data[i];
    }
    return ret_data;
}

static inline boolean validateSystemName(uint8_t *data, uint32_t len){
    if (len > FBL_DID_SYSTEM_NAME_BYTES_SIZE)
        return false;

    for(int i = 0; i < len; i++){
        // Directly stop at Null character
        if(data[i] != '\0' )
            return true;

        // Only allow from ASCII Space (32) to Tilde (126)
        if(data[i] < 32 || data[i] > 126)
            return false;
    }
    return true;
}

static boolean validateMemory(){
    boolean valid = true;

    // Check on the system Name
    if(!validateSystemName(memData.did_system_name, FBL_DID_SYSTEM_NAME_BYTES_SIZE))
        valid = false;

    // Check on valid date in BCD format [dd][MM][yy][HH][mm][ss]
    if( FBL_DID_PROGRAMMING_DATE_BYTES_SIZE == 6 && (
        memData.did_programming_date[0] > 0x31 || // Checking Day for range 0-31
        memData.did_programming_date[1] > 0x12 || // Checking Month for range 0-12
        memData.did_programming_date[2] > 0x99 || // Checking Year for range 0-99
        memData.did_programming_date[3] > 0x23 || // Checking Hour for range 0-23
        memData.did_programming_date[4] > 0x59 || // Checking Minute for range 0-59
        memData.did_programming_date[5] > 0x59 )) // Checking Second for range 0-59
        valid = false;

    return valid;
}

//============================================================================
// Init
//============================================================================

/**
 * This function reads all the data from the memory and fills the variables (TBD)
 */
void init_memory(void){

    // Reading from memory
    uint32_t len = sizeof(memData);
    uint8_t *dataRead = flashRead((uint32)DID_DATA_FLASH_ADDR, len);

    // Copy content to Variables
    write_to_variable(len, dataRead, (uint8_t*)(&memData));
    free(dataRead);

    // Validate the Memory Based on Programming Date and System Name
    init = validateMemory();

    // Check Init
    if(!init){
        // Info: Size of default System name usually < 32 Byte
        uint8_t did_system_name[] = FBL_DID_SYSTEM_NAME_DEFAULT;
        write_to_variable(sizeof(did_system_name), did_system_name, memData.did_system_name);

        uint8_t did_programming_date[] = FBL_DID_PROGRAMMING_DATE_DEFAULT;
        write_to_variable(FBL_DID_PROGRAMMING_DATE_BYTES_SIZE, did_programming_date, memData.did_programming_date);

        uint8_t did_bl_key_address[] = FBL_DID_BL_KEY_ADDRESS_DEFAULT;
        write_to_variable(FBL_DID_BL_KEY_ADDRESS_BYTES_SIZE, did_bl_key_address, memData.did_bl_key_address);

        uint8_t did_bl_key_good_value[] = FBL_DID_BL_KEY_GOOD_VALUE_DEFAULT;
        write_to_variable(FBL_DID_BL_KEY_GOOD_VALUE_BYTES_SIZE, did_bl_key_good_value, memData.did_bl_key_good_value);

        uint8_t did_can_base_mask[] = FBL_DID_CAN_BASE_MASK_DEFAULT;
        write_to_variable(FBL_DID_CAN_BASE_MASK_BYTES_SIZE, did_can_base_mask, memData.did_can_base_mask);

        uint8_t did_can_id[] = FBL_DID_CAN_ID_DEFAULT;
        write_to_variable(FBL_DID_CAN_ID_BYTES_SIZE, did_can_id, memData.did_can_id);

        uint8_t did_bl_write_start_add_core0[] = FBL_DID_BL_WRITE_START_ADD_CORE0_DEFAULT;
        write_to_variable(FBL_DID_BL_WRITE_START_ADD_CORE0_BYTES_SIZE, did_bl_write_start_add_core0, memData.did_bl_write_start_add_core0);

        uint8_t did_bl_write_end_add_core0[] = FBL_DID_BL_WRITE_END_ADD_CORE0_DEFAULT;
        write_to_variable(FBL_DID_BL_WRITE_END_ADD_CORE0_BYTES_SIZE, did_bl_write_end_add_core0, memData.did_bl_write_end_add_core0);

        uint8_t did_bl_write_start_add_core1[] = FBL_DID_BL_WRITE_START_ADD_CORE1_DEFAULT;
        write_to_variable(FBL_DID_BL_WRITE_START_ADD_CORE1_BYTES_SIZE, did_bl_write_start_add_core1, memData.did_bl_write_start_add_core1);

        uint8_t did_bl_write_end_add_core1[] = FBL_DID_BL_WRITE_END_ADD_CORE1_DEFAULT;
        write_to_variable(FBL_DID_BL_WRITE_END_ADD_CORE1_BYTES_SIZE, did_bl_write_end_add_core1, memData.did_bl_write_end_add_core1);

        uint8_t did_bl_write_start_add_core2[] = FBL_DID_BL_WRITE_START_ADD_CORE2_DEFAULT;
        write_to_variable(FBL_DID_BL_WRITE_START_ADD_CORE2_BYTES_SIZE, did_bl_write_start_add_core2, memData.did_bl_write_start_add_core2);

        uint8_t did_bl_write_end_add_core2[] = FBL_DID_BL_WRITE_END_ADD_CORE2_DEFAULT;
        write_to_variable(FBL_DID_BL_WRITE_END_ADD_CORE2_BYTES_SIZE, did_bl_write_end_add_core2, memData.did_bl_write_end_add_core2);
    }
}

//============================================================================
// Identification
//============================================================================

uint32_t getID(void){
    return (uint32_t)(FBLCAN_BASE_ADDRESS) | ((uint16_t)memData.did_can_id[0] << 12 | (uint16_t)(memData.did_can_id[1])<<4);
}


//============================================================================
// Read/Write Memory
//============================================================================


uint8_t readMemory(uint32_t address, uint16_t len, uint8_t* data){
    // TODO implement
    // returns value of read bytes (len = numberOfBytes)
    // data already allocated before function call, only needs to be filled with read memory
    for (uint8_t i = 0; i < len; i++){
        data[i] = i;
    }
    return 0;
    // returns 0: no error
    // returns something else: error occurred
}



uint8_t* readData(uint16_t identifier, uint8_t* len, uint8_t* nrc){
    *nrc = 0;

    switch(identifier){
        case FBL_DID_SYSTEM_NAME:
            *len = 0;
            while(*len < FBL_DID_SYSTEM_NAME_BYTES_SIZE && memData.did_system_name[*len] != '\0')
                ++*len;
            uint8_t* data = prepare_system_name_message(len, memData.did_system_name);
            return data;

        case FBL_DID_PROGRAMMING_DATE:
            *len = FBL_DID_PROGRAMMING_DATE_BYTES_SIZE;
            return prepare_message(len, memData.did_programming_date);

        case FBL_DID_BL_KEY_ADDRESS:
            *len = FBL_DID_BL_KEY_ADDRESS_BYTES_SIZE;
            return prepare_message(len, memData.did_bl_key_address);

        case FBL_DID_BL_KEY_GOOD_VALUE:
            *len = FBL_DID_BL_KEY_GOOD_VALUE_BYTES_SIZE;
            return prepare_message(len, memData.did_bl_key_good_value);

        case FBL_DID_CAN_BASE_MASK:
            *len = FBL_DID_CAN_BASE_MASK_BYTES_SIZE;
            return prepare_message(len, memData.did_can_base_mask);

        case FBL_DID_CAN_ID:
            *len = FBL_DID_CAN_ID_BYTES_SIZE;
            return prepare_message(len, memData.did_can_id);

        case FBL_DID_BL_WRITE_START_ADD_CORE0:
            *len = FBL_DID_BL_WRITE_START_ADD_CORE0_BYTES_SIZE;
            return prepare_message(len, memData.did_bl_write_start_add_core0);

        case FBL_DID_BL_WRITE_END_ADD_CORE0:
            *len = FBL_DID_BL_WRITE_END_ADD_CORE0_BYTES_SIZE;
            return prepare_message(len, memData.did_bl_write_end_add_core0);


        case FBL_DID_BL_WRITE_START_ADD_CORE1:
            *len = FBL_DID_BL_WRITE_START_ADD_CORE1_BYTES_SIZE;
            return prepare_message(len, memData.did_bl_write_start_add_core1);

        case FBL_DID_BL_WRITE_END_ADD_CORE1:
            *len = FBL_DID_BL_WRITE_END_ADD_CORE1_BYTES_SIZE;
            return prepare_message(len, memData.did_bl_write_end_add_core1);


        case FBL_DID_BL_WRITE_START_ADD_CORE2:
            *len = FBL_DID_BL_WRITE_START_ADD_CORE2_BYTES_SIZE;
            return prepare_message(len, memData.did_bl_write_start_add_core2);

        case FBL_DID_BL_WRITE_END_ADD_CORE2:
            *len = FBL_DID_BL_WRITE_END_ADD_CORE2_BYTES_SIZE;
            return prepare_message(len, memData.did_bl_write_end_add_core2);

        default:
            break;
    }

    // Negative Response Code
    *len = 0;
    *nrc = FBL_RC_SUB_FUNC_NOT_SUPPORTED;
    return 0; // return 0 on error
}

uint8_t writeData(uint16_t identifier, uint8_t* data, uint8_t len){
    // Store values to local variables if we have enough space to keep answer times short -> Write in case of writeData
    switch(identifier){
        case FBL_DID_SYSTEM_NAME:
            if(len > FBL_DID_SYSTEM_NAME_BYTES_SIZE)
                return FBL_RC_REQUEST_OUT_OF_RANGE;

            if(!validateSystemName(data,  len))
                return FBL_RC_INCORRECT_MSG_LEN_OR_INV_FORMAT;

            write_to_variable(len, data, memData.did_system_name);
            break;

        case FBL_DID_PROGRAMMING_DATE:
            if(len != FBL_DID_PROGRAMMING_DATE_BYTES_SIZE)
                return FBL_RC_REQUEST_OUT_OF_RANGE;
            write_to_variable(len, data, memData.did_programming_date);
            break;

        case FBL_DID_BL_KEY_ADDRESS:
            if(len != FBL_DID_BL_KEY_ADDRESS_BYTES_SIZE)
                return FBL_RC_REQUEST_OUT_OF_RANGE;
            write_to_variable(len, data, memData.did_bl_key_address);
            break;

        case FBL_DID_BL_KEY_GOOD_VALUE:
            if(len != FBL_DID_BL_KEY_GOOD_VALUE_BYTES_SIZE)
                return FBL_RC_REQUEST_OUT_OF_RANGE;
            write_to_variable(len, data, memData.did_bl_key_good_value);
            break;

        case FBL_DID_CAN_BASE_MASK:
            if(len != FBL_DID_CAN_BASE_MASK_BYTES_SIZE)
                return FBL_RC_REQUEST_OUT_OF_RANGE;
            write_to_variable(len, data, memData.did_can_base_mask);
            break;

        case FBL_DID_CAN_ID:
            if(len != FBL_DID_CAN_ID_BYTES_SIZE)
                return FBL_RC_REQUEST_OUT_OF_RANGE;
            write_to_variable(len, data, memData.did_can_id);
            break;

        case FBL_DID_BL_WRITE_START_ADD_CORE0:
            if(len != FBL_DID_BL_WRITE_START_ADD_CORE0_BYTES_SIZE)
                return FBL_RC_REQUEST_OUT_OF_RANGE;
            write_to_variable(len, data, memData.did_bl_write_start_add_core0);
            break;

        case FBL_DID_BL_WRITE_END_ADD_CORE0:
            if(len != FBL_DID_BL_WRITE_END_ADD_CORE0_BYTES_SIZE)
                return FBL_RC_REQUEST_OUT_OF_RANGE;
            write_to_variable(len, data, memData.did_bl_write_end_add_core0);
            break;


        case FBL_DID_BL_WRITE_START_ADD_CORE1:
            if(len != FBL_DID_BL_WRITE_START_ADD_CORE1_BYTES_SIZE)
                return FBL_RC_REQUEST_OUT_OF_RANGE;
            write_to_variable(len, data, memData.did_bl_write_start_add_core1);
            break;

        case FBL_DID_BL_WRITE_END_ADD_CORE1:
            if(len != FBL_DID_BL_WRITE_END_ADD_CORE1_BYTES_SIZE)
                return FBL_RC_REQUEST_OUT_OF_RANGE;
            write_to_variable(len, data, memData.did_bl_write_end_add_core1);
            break;


        case FBL_DID_BL_WRITE_START_ADD_CORE2:
            if(len != FBL_DID_BL_WRITE_START_ADD_CORE2_BYTES_SIZE)
                return FBL_RC_REQUEST_OUT_OF_RANGE;
            write_to_variable(len, data, memData.did_bl_write_start_add_core2);
            break;

        case FBL_DID_BL_WRITE_END_ADD_CORE2:
            if(len != FBL_DID_BL_WRITE_END_ADD_CORE2_BYTES_SIZE)
                return FBL_RC_REQUEST_OUT_OF_RANGE;
            write_to_variable(len, data, memData.did_bl_write_end_add_core2);
            break;

        default: // Negative Response Code
            return FBL_RC_SUB_FUNC_NOT_SUPPORTED;
    }

    // Finally store the changed values to flash
    flashWrite((uint32)DID_DATA_FLASH_ADDR, (uint32_t*)(&memData), sizeof(memData) / sizeof(uint32_t));

    return 0; // return 0 on success
}
