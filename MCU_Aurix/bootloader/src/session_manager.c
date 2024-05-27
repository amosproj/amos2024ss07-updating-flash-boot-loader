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

uint8_t isAuthorized(void){
    // TODO needs to be implemented
    return 1; // TODO check if auth
}

uint8_t generateSeed(uint8_t* seed){
    // seed needs to be SEED_LENGTH bytes long
    // TODO needs to be implemented
    return 1;
}

uint8_t verifyKey(uint8_t* key, uint8_t key_len){
    // TODO implement

    // returns 0: key matches, returns something else: key not matching key we calculated from seed
    return 0;
}

uint8_t getSession(void){
    // TODO getSession just a mockup, not really implemented yet!
    return FBL_DIAG_SESSION_DEFAULT;
}
