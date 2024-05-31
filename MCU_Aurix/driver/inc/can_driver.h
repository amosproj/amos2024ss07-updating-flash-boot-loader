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

/**********/
/*INCLUDES*/
/*********/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "Bsp.h"


int canTransmitMessage(uint32_t canMessageID, uint8_t* data, size_t size);
void canIsrRxHandler();

#endif /*CAN_DRIVER_H*/


