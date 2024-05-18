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

//ISOTP: Added Debug data2 to test isoTP
#define CAN_DEBUG_DATA2  (uint8_t)0xBABAB055
#define TX_DATA_LOW_WORD            (uint32_t)0xC0CAC01A      /* Define CAN data lower word to be transmitted         */
#define TX_DATA_HIGH_WORD           (uint32_t)0xBA5EBA11      /* Define CAN data higher word to be transmitted        */

/**********/
/*INCLUDES*/
/*********/
#include <stdio.h>
#include <string.h>
#include <stdint.h>

//ISOTP: testing different headers
//void canTransmitMessage(uint32_t canMessageID, uint8_t *low_word_data, size_t low_word_size, uint8_t *high_word_data, size_t high_word_size); //oder uint8_t*
//void canTransmitMessage(uint32_t canMessageID, uint32_t low_word, uint32_t high_word); //oder uint8_t*
int canTransmitMessage(uint32_t canMessageID, uint8_t* data, size_t size); //oder uint8_t*
void canIsrRxHandler();

#endif /*CAN_DRIVER_H*/


