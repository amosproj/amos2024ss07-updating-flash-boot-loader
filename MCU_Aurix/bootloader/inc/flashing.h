// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>

//============================================================================
// Name        : flashing.h
// Author      : Dorothea Ehrl
// Version     : 0.1
// Copyright   : MIT
// Description : Manages flash data
//============================================================================

#ifndef BOOTLOADER_INC_FLASHING_H_
#define BOOTLOADER_INC_FLASHING_H_

#include "Ifx_Types.h"

// TODO not all of them should return void
void getFlashConfiguration();
void writeToFlash(uint8 data);
void getDataFromFlash();


#endif /* BOOTLOADER_INC_FLASHING_H_ */
