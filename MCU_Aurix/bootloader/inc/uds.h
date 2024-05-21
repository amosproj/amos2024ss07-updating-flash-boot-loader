// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>

//============================================================================
// Name        : uds.h
// Author      : Dorothea Ehrl
// Version     : 0.1
// Copyright   : MIT
// Description : Handles UDS messages from CAN bus
//============================================================================

#ifndef BOOTLOADER_INC_UDS_H_
#define BOOTLOADER_INC_UDS_H_

#include "Ifx_Types.h"

#include "uds_comm_spec.h"

void handleRXUDS(uint8* data, uint32 data_len);

#endif /* BOOTLOADER_INC_UDS_H_ */
