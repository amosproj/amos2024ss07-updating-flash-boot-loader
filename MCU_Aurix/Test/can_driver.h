// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Sebastian Rodriguez <r99@melao.de>

//============================================================================
// Name        : can_driver.h
// Author      : Sebastian Rodriguez
// Version     : 0.1
// Copyright   : MIT
// Description : Header for CAN Driver
//============================================================================

#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#define CAN_DEBUG_ID    (uint32)0x777
#define CAN_DEBUG_DATA  (uint64_t)0xBABAB055

/**********/
/*INCLUDES*/
/*********/
#include <stdio.h>
#include <string.h>
#include <stdint.h>

void canTransmitMessage(uint32_t canMessageID, uint64_t data, uint64_t len); //oder uint8_t*
void canIsrRxHandler();
void initLeds(void);

#endif /*CAN_DRIVER_H*/


