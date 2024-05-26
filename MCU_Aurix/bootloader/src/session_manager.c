// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>

//============================================================================
// Name        : session_manager.h
// Author      : Dorothea Ehrl
// Version     : 0.1
// Copyright   : MIT
// Description : Manages bootloader session including auth
//============================================================================

#include "session_manager.h"
#include "uds_comm_spec.h"

uint8 getSession(void){
    // TODO getSession just a mockup, not really implemented yet!
    return FBL_DIAG_SESSION_DEFAULT;
}
