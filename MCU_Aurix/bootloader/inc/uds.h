// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>, Sebastian Rodriguez <r99@melao.de>

//============================================================================
// Name        : uds.h
// Author      : Dorothea Ehrl, Sebastian Rodriguez
// Version     : 0.1
// Copyright   : MIT
// Description : Handles UDS messages from CAN bus
//============================================================================

#ifndef BOOTLOADER_INC_UDS_H_
#define BOOTLOADER_INC_UDS_H_

#include "Ifx_Types.h"

#include "uds_comm_spec.h"

void uds_handleRX(uint8* data, uint32 data_len);

void uds_diagnostic_session_control(void);
void uds_ecu_reset(uint8 reset_type);
void uds_security_access(u_int8_t reset_type);
void uds_tester_present(void);
void uds_read_data_by_identifier(uint16 did);
void uds_read_memory_by_address(uint8* address);
void uds_write_data_by_identifier(uint8* data);
void uds_request_download(void);
void uds_request_upload(void);
void uds_transfer_data(uint8* data);
void uds_request_transfer_exit();
void uds_neg_response(uint8* data);

#endif /* BOOTLOADER_INC_UDS_H_ */
