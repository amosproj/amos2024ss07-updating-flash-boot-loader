// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>
// SPDX-FileCopyrightText: 2024 Sebastian Rodriguez <r99@melao.de>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>
//============================================================================
// Name        : session_manager.h
// Author      : Dorothea Ehrl, Sebastian Rodriguez, Michael Bauer, Wiktor Pilarczyk
// Version     : 0.2
// Copyright   : MIT
// Description : Manages bootloader session including auth
//============================================================================

#ifndef BOOTLOADER_INC_SESSION_MANAGER_H_
#define BOOTLOADER_INC_SESSION_MANAGER_H_

#define SESSION_TIMEOUT_MS          (5000)

#include <stdint.h>

#include "Ifx_Types.h"

//============================================================================
// Init
//============================================================================

void init_session_manager(void);

//============================================================================
// Session Handling
//============================================================================

boolean setSession(uint8_t session);
uint8_t getSession(void);
void sessionControl(void);
uint8_t SIDallowedInCurrentSession(uint8_t SID);

//============================================================================
// Authentification
//============================================================================

uint8_t generateSeed(uint8_t* seed);
uint8_t verifyKey(uint8_t* key, uint8_t key_len);
void resetAuthentication(void);
boolean isAuthorized(void);

//============================================================================
// Reset
//============================================================================

boolean isResetTypeAvailable(uint8_t reset_type);
void resetECU(uint8_t reset_type);

#endif /* BOOTLOADER_INC_SESSION_MANAGER_H_ */
