// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Sebastian Rodriguez <r99@melao.de>

//============================================================================
// Name        : can_driver.c
// Author      : Sebastian Rodriguez
// Version     : 0.1
// Copyright   : MIT
// Description : C File for CAN Driver
//============================================================================

#include "can_driver.h"

/**
 * Interrupt Service Routine (ISR) is called when RX Interrupt is generated
 * Calls function to execute on Data Read in CAN Message
 * @param processDataFunction Pointer to function that processes Data read in CAN Message
*/
void canIsrRxHandler(void (*processDataFunction)(void*));

/**
 * Initialize CAN Module and Node
*/
void initCanDriver(){

}

/**
 * Transmits a CAN Message: Initialize new TX and RX message, TX is transmitted
 * @param canMessageID ID of CAN Message for Prio in BUS
 * @param TXLowDataWord First 4 Bytes in TX CAN Message
 * @param TXHighDataWord Last 4 Bytes in TX CAN Message
*/
void transmitCanMessage(uint32 canMessageID, uint32 TXLowDataWord, uint32 TXHighDataWord){

}


