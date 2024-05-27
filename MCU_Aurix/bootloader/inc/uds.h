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

#include <stdint.h>

#include "uds_comm_spec.h"

#define SEED_LENGTH     5

void uds_handleRX(uint8_t* data, uint32_t data_len);

void uds_diagnostic_session_control(void);
void uds_ecu_reset(uint8_t reset_type);
void uds_security_access(uint8_t request_type, uint8_t *key, uint8_t key_len);
void uds_tester_present(void);
void uds_read_data_by_identifier(uint16_t did);
void uds_read_memory_by_address(uint32_t address, uint16_t noBytesToRead);
void uds_write_data_by_identifier(uint8_t* data);
void uds_request_download(void);
void uds_request_upload(void);
void uds_transfer_data(uint8_t* data);
void uds_request_transfer_exit();
void uds_neg_response(uint8_t reg_sid, uint8_t neg_code);

#endif /* BOOTLOADER_INC_UDS_H_ */
