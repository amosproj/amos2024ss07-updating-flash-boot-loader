// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : flashing.h
// Author      : Dorothea Ehrl, Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Manages flash data
//============================================================================

#ifndef BOOTLOADER_INC_FLASHING_H_
#define BOOTLOADER_INC_FLASHING_H_

#define FLASHING_FLASHING_ENDIANNESS    (0)     // 0 = Big-endian, 1=Little-endian; // TODO: Other format necessary? Big vs. Little Endian

#include "Ifx_Types.h"
#include <stdint.h>

void flashingInit(void);

uint8_t flashingRequestDownload(uint32_t address, uint32_t data_len);
uint8_t flashingRequestUpload(uint32_t address, uint32_t data_len);
uint8_t flashingTransferData(uint32_t address, uint8_t* data, uint32_t data_len);
uint8_t flashingTransferExit(uint32_t address);

uint32_t flashingGetFlashBufferSize();

// TODO not all of them should return void
void getFlashConfiguration();
void writeToFlash(uint8 data);
void getDataFromFlash();


#endif /* BOOTLOADER_INC_FLASHING_H_ */
