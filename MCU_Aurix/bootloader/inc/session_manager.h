// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>

//============================================================================
// Name        : session_manager.h
// Author      : Dorothea Ehrl
// Version     : 0.1
// Copyright   : MIT
// Description : Manages bootloader session including auth
//============================================================================

#ifndef BOOTLOADER_INC_SESSION_MANAGER_H_
#define BOOTLOADER_INC_SESSION_MANAGER_H_

#include <stdint.h>

#include "Ifx_Types.h"

// TODO should not all return void
void setSession(void);
uint8_t generateSeed(uint8_t* seed);
uint8_t verifyKey(uint8_t* key, uint8_t key_len);
void authenticate(void);
uint8_t isAuthorized();
uint8_t getSession(void);
void sessionControl(void);
void resetECU(void);

#endif /* BOOTLOADER_INC_SESSION_MANAGER_H_ */
