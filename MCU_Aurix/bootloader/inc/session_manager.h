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

#include "Ifx_Types.h"

// TODO should not all return void
void setSession(void);
void authenticate(void);
uint8 getSession(void);
void sessionControl(void);
void resetECU(void);

#endif /* BOOTLOADER_INC_SESSION_MANAGER_H_ */
