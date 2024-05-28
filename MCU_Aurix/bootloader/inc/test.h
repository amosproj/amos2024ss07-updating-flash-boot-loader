// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText:  Sebastian Rodriguez <r99@melao.de>, 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>

//============================================================================
// Name        : session_manager.h
// Author      : Sebastian Rodriguez, Dorothea Ehrl
// Version     : 0.1
// Copyright   : MIT
// Description : Test header file
//============================================================================

#ifndef BOOTLOADER_INC_TEST_H_
#define BOOTLOADER_INC_TEST_H_
#include "uds.h"

void test_diagnostic_session_control(void);
void test_ecu_reset(void);
void test_security_access(void);
void test_tester_present(void);
void test_read_data_by_identifier(void);
void test_read_memory_by_address(void);
void test_write_data_by_identifier(void);
void test_request_download(void);
void test_request_upload(void);
void test_transfer_data(void);
void test_request_transfer_exit(void);
void test_neg_response(void);

#endif /* BOOTLOADER_INC_TEST_H_ */