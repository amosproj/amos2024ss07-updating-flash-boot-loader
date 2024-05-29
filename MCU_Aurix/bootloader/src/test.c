// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Sebastian Rodriguez <r99@melao.de>
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

//============================================================================
// Name        : test.c
// Author      : Sebastian Rodriguez, Dorothea Ehrl, Wiktor Pilarczyk
// Version     : 0.2
// Copyright   : MIT
// Description : Test implementation
//============================================================================

#include "test.h"

void test_diagnostic_session_control(void){
    //default session
    int len;   
    uint8_t* data = _create_diagnostic_session_control(&len, 0, FBL_DIAG_SESSION_DEFAULT);
    uds_handleRX(data, len);
    //programming session
    int len2;
    uint8_t *data2 = _create_diagnostic_session_control(&len2, 0, FBL_DIAG_SESSION_PROGRAMMING);
    uds_handleRX(data2, len2);
    free(data);
    free(data2);
}

void test_ecu_reset(void){
    int len;
    uint8_t* data = _create_ecu_reset(&len, 0, FBL_ECU_RESET_HARD);
    uds_handleRX(data, len);
    free(data);
}

void test_security_access(void){
    //Access Seed
    int len;
    uint8_t* data = _create_security_access(&len, 0, FBL_SEC_ACCESS_SEED, 0,0);
    uds_handleRX(data, len);
    free(data);
}

void test_tester_present(void){
    int len;
    uint8_t* data = _create_tester_present(&len, 0, FBL_TESTER_PRES_WITH_RESPONSE);
    uds_handleRX(data, len);
    free(data);
}

void test_read_data_by_identifier(void){
    //System Name
    int len;
    uint8_t *data = _create_read_data_by_ident(&len, 0, FBL_DID_SYSTEM_NAME, 0, 0);
    uds_handleRX(data, len);
    free(data);
}

void test_read_memory_by_address(void){
}    

void test_write_data_by_identifier(void){
}

void test_request_download(void){
}

void test_request_upload(void){
}

void test_transfer_data(void){
}

void test_request_transfer_exit(void){
}