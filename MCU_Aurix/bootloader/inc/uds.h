// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Sebastian Rodriguez <r99@melao.de>

//============================================================================
// Name        : uds.h
// Author      : Sebastian Rodriguez
// Version     : 0.1
// Copyright   : MIT
// Description : UDS Layer
//============================================================================

#ifndef UDS_H_
#define UDS_H_

#include "uds_comm_spec.h"

void uds_diagnostic_session_control(uint8_t session);
void uds_ecu_reset();
void uds_security_access();
void uds_tester_present();
void uds_read_data_by_identifier(uint8_t identifier);
void uds_read_memory_by_address(uint8_t* address);
void uds_write_data_by_identifier(uint8_t* data);
void uds_request_download();
void uds_request_upload();
void uds_transfer_data(uint8_t* data);
void uds_request_transfer_exit();
void uds_neg_response(uint8_t* data);

#endif