// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>
// SPDX-FileCopyrightText: 2024 Sebastian Rodriguez <r99@melao.de>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>
//============================================================================
// Name        : session_manager.h
// Author      : Dorothea Ehrl, Sebastian Rodriguez, Michael Bauer
// Version     : 0.2
// Copyright   : MIT
// Description : Manages bootloader session including auth
//============================================================================

#include "session_manager.h"
#include "uds_comm_spec.h"

uint8_t session;
boolean authenticated;

//============================================================================
// Init
//============================================================================

void init(){
    session = FBL_DIAG_SESSION_DEFAULT;
    authenticated = 1; // Currently no authentification necessary
}

//============================================================================
// Session Handling
//============================================================================
uint8_t getSession(void){
    return session;
}

uint8_t setSession(uint8_t session_to_set){

    if(session != FBL_DIAG_SESSION_DEFAULT && session != FBL_DIAG_SESSION_PROGRAMMING)
        return FBL_RC_SUB_FUNC_NOT_SUPPORTED;

    session = session_to_set;
    return 0;
}

/**
 * Function to check if SID is allowed in current session
 *
 * Return 0: if allowed
 * Return NRC: if not allowed, Negative Response Code can directly be forwarded
 */
uint8_t SIDallowedInCurrentSession(uint8_t SID){

    if(getSession() == FBL_DIAG_SESSION_DEFAULT){
        switch (SID){
            case FBL_SECURITY_ACCESS:
            case FBL_READ_MEMORY_BY_ADDRESS:
            case FBL_WRITE_DATA_BY_IDENTIFIER:
            case FBL_REQUEST_DOWNLOAD:
            case FBL_REQUEST_UPLOAD:
            case FBL_TRANSFER_DATA:
            case FBL_REQUEST_TRANSFER_EXIT:
                return FBL_RC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
            default:
                break;
        }
    }
    else if(getSession() == FBL_DIAG_SESSION_PROGRAMMING ){
        switch (SID){
            case FBL_SECURITY_ACCESS:
            case FBL_READ_MEMORY_BY_ADDRESS:
            case FBL_WRITE_DATA_BY_IDENTIFIER:
            case FBL_REQUEST_DOWNLOAD:
            case FBL_REQUEST_UPLOAD:
            case FBL_TRANSFER_DATA:
            case FBL_REQUEST_TRANSFER_EXIT:
                if(!isAuthorized())
                    return FBL_RC_SECURITY_ACCESS_DENIED;
                break;
            default:
                break;
        }
    }
    return 0;
}

//============================================================================
// Authentification
//============================================================================

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

uint8_t isAuthorized(void){
    return authenticated;
}

//============================================================================
// Reset
//============================================================================

uint8_t isResetTypeAvailable(uint8_t reset_type){
    switch (reset_type){
        case FBL_ECU_RESET_POWERON:
        case FBL_ECU_RESET_COLD_POWERON:
        case FBL_ECU_RESET_WARM_POWERON:
            return 0;
        default:
            return FBL_RC_SUB_FUNC_NOT_SUPPORTED;
    }
}

void resetECU(uint8_t reset_type){
    // TODO implement
}

