// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>

//============================================================================
// Name        : memory.h
// Author      : Dorothea Ehrl
// Version     : 0.1
// Copyright   : MIT
// Description : Manages writing and returning data in memory
//============================================================================

#ifndef BOOTLOADER_INC_MEMORY_H_
#define BOOTLOADER_INC_MEMORY_H_

#include <stdint.h>

#include "Ifx_Types.h"

// TODO not all of them should return void and datatype might be wrong
void getWriteableMemory();
void getIdentification();
void setID(uint8 id);
void readData(uint8 identifier);
uint8_t readMemory(uint32_t address, uint16_t len, uint8_t* data);
void writeData(uint8 identifier, uint8 data);


#endif /* BOOTLOADER_INC_MEMORY_H_ */
