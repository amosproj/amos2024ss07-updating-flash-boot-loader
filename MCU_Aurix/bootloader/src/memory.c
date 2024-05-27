// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>

//============================================================================
// Name        : memory.c
// Author      : Dorothea Ehrl
// Version     : 0.1
// Copyright   : MIT
// Description : Manages writing and returning data in memory
//============================================================================

#include "memory.h"

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
