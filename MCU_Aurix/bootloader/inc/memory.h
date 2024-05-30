// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>
// SPDX-FileCopyrightText: 2024 Sebastian Rodriguez <r99@melao.de>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@faud.de>

//============================================================================
// Name        : memory.h
// Author      : Dorothea Ehrl, Sebastian Rodriguez, Michael Bauer
// Version     : 0.2
// Copyright   : MIT
// Description : Manages writing and returning data in memory
//============================================================================

#ifndef BOOTLOADER_INC_MEMORY_H_
#define BOOTLOADER_INC_MEMORY_H_

#include <stdint.h>

#include "Ifx_Types.h"

#define FBL_DID_SYSTEM_NAME_BYTES_SIZE                              (32)
#define FBL_DID_PROGRAMMING_DATE_BYTES_SIZE                         (3)
#define FBL_DID_BL_KEY_ADDRESS_BYTES_SIZE                           (4)
#define FBL_DID_BL_KEY_GOOD_VALUE_BYTES_SIZE                        (4)
#define FBL_DID_CAN_BASE_MASK_BYTES_SIZE                            (2)
#define FBL_DID_CAN_ID_BYTES_SIZE                                   (2)
#define FBL_DID_BL_WRITE_START_ADD_CORE0_BYTES_SIZE                 (4)
#define FBL_DID_BL_WRITE_END_ADD_CORE0_BYTES_SIZE                   (4)
#define FBL_DID_BL_WRITE_START_ADD_CORE1_BYTES_SIZE                 (4)
#define FBL_DID_BL_WRITE_END_ADD_CORE1_BYTES_SIZE                   (4)
#define FBL_DID_BL_WRITE_START_ADD_CORE2_BYTES_SIZE                 (4)
#define FBL_DID_BL_WRITE_END_ADD_CORE2_BYTES_SIZE                   (4)


//============================================================================
// Init
//============================================================================
void init_memory();

//============================================================================
// Identification
//============================================================================

uint32_t getID();

//============================================================================
// Read/Write Memory
//============================================================================

uint8_t readMemory(uint32_t address, uint16_t len, uint8_t* data);
uint8_t* readData(uint16_t identifier, uint8_t* len, uint8_t* nrc);
uint8_t writeData(uint16_t identifier, uint8_t* data, uint8_t len);


#endif /* BOOTLOADER_INC_MEMORY_H_ */
