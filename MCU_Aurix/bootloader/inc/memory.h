// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>
// SPDX-FileCopyrightText: 2024 Sebastian Rodriguez <r99@melao.de>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@faud.de>

//============================================================================
// Name        : memory.h
// Author      : Dorothea Ehrl, Sebastian Rodriguez, Michael Bauer
// Version     : 0.3
// Copyright   : MIT
// Description : Manages writing and returning data in memory
//============================================================================

#ifndef BOOTLOADER_INC_MEMORY_H_
#define BOOTLOADER_INC_MEMORY_H_

#include <stdint.h>

#include "Ifx_Types.h"

#define DID_DATA_FLASH_ADDR                                         0xAF000000
#define FBL_STRUCTURE_VERSION                                       (4)
#define FBL_STRUCTURE_DATA_FLASH_STRUCTURE_VERSION                  {0x00, 0x00, 0x00, 0x01} // INFO: Change if Structure or Default Data changed, forces loading of Default config

// Size in uint8_t bytes
// Info: Changing of size may effect other modules -> Flashing
#define FBL_DID_APP_ID_BYTES_SIZE                                   (32)
#define FBL_DID_SYSTEM_NAME_BYTES_SIZE                              (32)
#define FBL_DID_PROGRAMMING_DATE_BYTES_SIZE                         (6)
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
#define FBL_DID_BL_WRITE_START_ADD_ASW_KEY_BYTES_SIZE               (4)
#define FBL_DID_BL_WRITE_END_ADD_ASW_KEY_BYTES_SIZE                 (4)
#define FBL_DID_BL_WRITE_START_ADD_CAL_DATA_BYTES_SIZE              (4)
#define FBL_DID_BL_WRITE_END_ADD_CAL_DATA_BYTES_SIZE                (4)


// Size need to match above mentioned defines
#define FBL_DID_APP_ID_DEFAULT                                      "amos basic"
#define FBL_DID_SYSTEM_NAME_DEFAULT                                 "AMOS FBL 24"
#define FBL_DID_PROGRAMMING_DATE_DEFAULT                            {0x17, 0x04, 0x24, 0x10, 0x15, 0x00}
#define FBL_DID_BL_KEY_ADDRESS_DEFAULT                              {0xA0, 0x4F, 0x80, 0x09}
#define FBL_DID_BL_KEY_GOOD_VALUE_DEFAULT                           {0x93, 0x86, 0xC3, 0xA5}
#define FBL_DID_CAN_BASE_MASK_DEFAULT                               {0x0F, 0x24}
#define FBL_DID_CAN_ID_DEFAULT                                      {0x00, 0x01}
#define FBL_DID_BL_WRITE_START_ADD_CORE0_DEFAULT                    {0xA0, 0x09, 0x00, 0x00}
#define FBL_DID_BL_WRITE_END_ADD_CORE0_DEFAULT                      {0xA0, 0x1F, 0xFF, 0xFF}
#define FBL_DID_BL_WRITE_START_ADD_CORE1_DEFAULT                    {0xA0, 0x30, 0x40, 0x00}
#define FBL_DID_BL_WRITE_END_ADD_CORE1_DEFAULT                      {0xA0, 0x4F, 0x7F, 0xFF}
#define FBL_DID_BL_WRITE_START_ADD_CORE2_DEFAULT                    {0xFF, 0xFF, 0xFF, 0xFF}
#define FBL_DID_BL_WRITE_END_ADD_CORE2_DEFAULT                      {0xFF, 0xFF, 0xFF, 0xFF}
#define FBL_DID_BL_WRITE_START_ADD_ASW_KEY_DEFAULT                  {0xA0, 0x4F, 0x80, 0x00}
#define FBL_DID_BL_WRITE_END_ADD_ASW_KEY_DEFAULT                    {0xA0, 0x4F, 0xBF, 0xFF}
#define FBL_DID_BL_WRITE_START_ADD_CAL_DATA_DEFAULT                 {0xA0, 0x4F, 0xC0, 0x00}
#define FBL_DID_BL_WRITE_END_ADD_CAL_DATA_DEFAULT                   {0xA0, 0x4F, 0xFF, 0xFF}

//KEY
#define KEY_ADDRESS                                                 0xA04F8009
#define KEY_GOOD_VALUE                                              0x9386C3A5
//============================================================================
// Init
//============================================================================
void init_memory(void);

//============================================================================
// Identification
//============================================================================

uint32_t getID(void);

//============================================================================
// Read/Write Memory
//============================================================================

uint8_t readMemory(uint32_t address, uint16_t len, uint8_t* data);
uint8_t* readData(uint16_t identifier, uint8_t* len, uint8_t* nrc);
uint8_t writeData(uint16_t identifier, uint8_t* data, uint8_t len);


#endif /* BOOTLOADER_INC_MEMORY_H_ */
