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

void (*processDataFunction)(void*);

canType g_can //Global control struct
/*Interrupts*/
IFX_INTERRUPT(canIsrTxHandler, 0, INTERRUPT_PRIO_TX);
IFX_INTERRUPT(canIsrRxHandler, 0, INTERRUP_PRIO_RX);

void canIsrTxHandler(void){
    IfxCan_Node_clearInterruptFlag(g_can.canSrcNode.node, IfxCan_Interrupt_transmissionCompleted); //Just clears the Interrupt 
}

/**
 * Interrupt Service Routine (ISR) is called when RX Interrupt is generated
 * Calls function to execute on Data Read in CAN Message
 * @param processDataFunction Pointer to function that processes Data read in CAN Message
*/
void canIsrRxHandler(){
    IfxCan_Node_clearInterruptFlag(g_can.canDstNode.node, IfxCan_Interrupt_messageStoredToDedicatedRxBuffer); /*Clear Message Stored Flag*/
    IfxCan_Can_readMessage(&g_mcmcan.canDstNode, &g_mcmcan.rxMsg, g_mcmcan.rxData); //Read Message
    if (processDataFunction != NULL)
    {
        //Call Function
    }

    
}

void initSrcNode(){
    //TODO: Change Loopback mode 
    IfxCan_Can_initNodeConfig(&g_can.canNodeConfig, &g_can.canModule);

    g_can.canNodeConfig.busLoopbackEnabled = TRUE;                               /*Loopback Mode (no external pins)*/
    g_can.canNodeConfig.nodeId = IfxCan_NodeId_0;                                /*ID = 0*/

    g_can.canNodeConfig.frame.type = IfxCan_FrameType_transmit;                  /*Frame is a transmitting one*/

    g_can.canNodeConfig.interruptConfig.transmissionCompletedEnabled = TRUE;     /*Raises Interrupt when transmition is done*/
    g_can.canNodeConfig.interruptConfig.traco.priority = ISR_PRIORITY_CAN_TX;    /*Prio*/
    g_can.canNodeConfig.interruptConfig.traco.interruptLine = IfxCan_InterruptLine_0; /*Interrupt line 0*/
    g_can.canNodeConfig.interruptConfig.traco.typeOfService = IfxSrc_Tos_cpu0;       /*On CPU0*/

    IfxCan_Can_initNode(&g_can.canSrcNode, &g_can.canNodeConfig);             /*INIT Node with this Config*/
}

void initDstNode(){
    //TODO: Change Loopback mode when PINS are used
    IfxCan_Can_initNodeConfig(&g_can.canNodeConfig, &g_can.canModule);                  /*Default Config*/

    g_can.canNodeConfig.busLoopbackEnabled = TRUE;                                      /*Loopbackmode*/
    g_can.canNodeConfig.nodeId = IfxCan_NodeId_1;                                       /*ID = 1*/

    g_can.canNodeConfig.frame.type = IfxCan_FrameType_receive;                          /*Receiving Frame*/

    g_can.canNodeConfig.interruptConfig.messageStoredToDedicatedRxBufferEnabled = TRUE; /*Raise Interrupt when msg is stored in RX Buffer*/
    g_can.canNodeConfig.interruptConfig.reint.priority = ISR_PRIORITY_CAN_RX;           /*Prio*/
    g_can.canNodeConfig.interruptConfig.reint.interruptLine = IfxCan_InterruptLine_1;   /*Interrupt Line 1*/
    g_can.canNodeConfig.interruptConfig.reint.typeOfService = IfxSrc_Tos_cpu0;          /*On CPU 0*/

    IfxCan_Can_initNode(&g_can.canDstNode, &g_can.canNodeConfig);                        /*INIT Node with this Config*/

    //TODO:Check if we need CAN Filter here
}

/**
 * Initialize CAN Module and Node
*/
void initCanDriver(void){
    IfxCan_Can_initModuleConfig(&g_can.canConfig, &MODULE_CAN0) /*LoadsDefault Config*/
    IfxCan_Can_initModule(&g_can.canModule, &g_can.canConfig) /*Init with default config*/

    initSrcNode();
    initDstNode();
}

/**
 * Transmits a CAN Message: Initialize new TX and RX message, TX is transmitted
 * @param canMessageID ID of CAN Message for Prio in BUS
 * @param TXLowDataWord First 4 Bytes in TX CAN Message
 * @param TXHighDataWord Last 4 Bytes in TX CAN Message
*/
void transmitCanMessage(uint32 canMessageID, uint32 TXLowDataWord, uint32 TXHighDataWord){
    
}


