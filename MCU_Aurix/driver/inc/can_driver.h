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

#define CAN_DEBUG_ID    (uint32_t)0x777
#define CAN_DEBUG_DATA  (uint64_t)0xBABAB055
#define TX_DATA_LOW_WORD            (uint32_t)0xC0CAC01A      /* Define CAN data lower word to be transmitted         */
#define TX_DATA_HIGH_WORD           (uint32_t)0xBA5EBA11      /* Define CAN data higher word to be transmitted        */

/**********/
/*INCLUDES*/
/*********/
#include <stdio.h>
#include <string.h>
#include <stdint.h>

void canTransmitMessage(uint32_t canMessageID, uint32_t lowWord, uint32_t highWord); //oder uint8_t*
void canIsrRxHandler();

#endif /*CAN_DRIVER_H*/


