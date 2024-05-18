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
#include "can_init.h"
#include "led_driver.h"


void (*processDataFunction)(void*);

//TODO: Implement the processDataFunction we want to use


canType g_can; //Global control struct


/*Interrupts*/
IFX_INTERRUPT(canIsrTxHandler, 0, INTERRUPT_PRIO_TX);
IFX_INTERRUPT(canIsrRxHandler, 0, INTERRUPT_PRIO_RX);

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
    IfxCan_Can_readMessage(&g_can.canDstNode, &g_can.rxMsg, g_can.rxData); //Read Message
    if (processDataFunction != NULL)
    {
        //Callback
    }

    if (DEBUGGING)
    {
        //LED 1 If Message ID TX and RX is same
        if (g_can.rxMsg.messageId == g_can.txMsg.messageId)
        {
            led_on(LED1);
        }
        //LED 2 if data TX and RX is the same
        if (g_can.rxData[0] == g_can.txData[0] )
        {
            //led_on(LED2);
        }
        
        
    }
    

    
}

void initSrcNode(){
    //TODO: Change Loopback mode 
    IfxCan_Can_initNodeConfig(&g_can.canNodeConfig, &g_can.canModule);

    g_can.canNodeConfig.busLoopbackEnabled = TRUE;                               /*Loopback Mode (no external pins)*/
    g_can.canNodeConfig.nodeId = IfxCan_NodeId_0;                                /*ID = 0*/

    g_can.canNodeConfig.frame.type = IfxCan_FrameType_transmit;                  /*Frame is a transmitting one*/

    g_can.canNodeConfig.interruptConfig.transmissionCompletedEnabled = TRUE;     /*Raises Interrupt when transmition is done*/
    g_can.canNodeConfig.interruptConfig.traco.priority = INTERRUPT_PRIO_TX;    /*Prio*/
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
    g_can.canNodeConfig.interruptConfig.reint.priority = INTERRUPT_PRIO_RX;           /*Prio*/
    g_can.canNodeConfig.interruptConfig.reint.interruptLine = IfxCan_InterruptLine_1;   /*Interrupt Line 1*/
    g_can.canNodeConfig.interruptConfig.reint.typeOfService = IfxSrc_Tos_cpu0;          /*On CPU 0*/

    IfxCan_Can_initNode(&g_can.canDstNode, &g_can.canNodeConfig);                        /*INIT Node with this Config*/

    //TODO:Check if we need CAN Filter here
}

/**
 * Initialize CAN Module and Node
*/
void canInitDriver(void){
    IfxCan_Can_initModuleConfig(&g_can.canConfig, &MODULE_CAN0); /*LoadsDefault Config*/
    IfxCan_Can_initModule(&g_can.canModule, &g_can.canConfig); /*Init with default config*/

    initSrcNode();
    initDstNode();
    IfxCan_Can_initMessage(&g_can.rxMsg); /*Init for RX Message*/
}

//ISOTP: Testing different headers for CAN messages

int canTransmitMessage(uint32_t canMessageID, uint8_t* data, size_t size){
    IfxCan_Can_initMessage(&g_can.txMsg);
    g_can.txMsg.messageId = canMessageID;

    //Not sure if necessary ~Leon
    // Ensure that the size of data does not exceed 8 bytes (32 bits)
    if (size > 8) {
        // Handle error: data size exceeds 4 bytes
        return -1;
    }

    // Initialize g_can.txData to zero
    g_can.txData[0] = 0;
    g_can.txData[1] = 0;

    // Copy up to 8 bytes of data into g_can.txData
    memcpy((uint8_t*)g_can.txData, data, size);


    //Sends CAN Message, only if BUS is empty
    while( IfxCan_Status_notSentBusy ==
           IfxCan_Can_sendMessage(&g_can.canSrcNode, &g_can.txMsg, g_can.txData))
    {


    }

    return 0;
}

/*
void canTransmitMessage(uint32_t canMessageID, uint32_t low_word, uint32_t high_word){
    IfxCan_Can_initMessage(&g_can.txMsg);
    g_can.txData[0] = low_word; //To transmit data
    g_can.txData[1] = high_word;
    g_can.txMsg.messageId = canMessageID;

    //Sends CAN Message, only if BUS is empty
    while( IfxCan_Status_notSentBusy ==
           IfxCan_Can_sendMessage(&g_can.canSrcNode, &g_can.txMsg, &g_can.txData[0]))
    {
    }
}
*/

/**
 * TODO: Check with Andi if this is ok
 * ATTENTION: CAN_sendMessage wants uint32_t data, but I thought we can send 8 bytes of data trough CAN? ~Leon
 * Transmits a CAN Message: Initialize new TX message, TX is transmitted
 * @param canMessageID ID of CAN Message for Prio in BUS
 * @param low_word
 * @param high_word
*/
/*
void canTransmitMessage(uint32_t canMessageID, uint8_t *low_word_data, size_t low_word_size, uint8_t *high_word_data, size_t high_word_size){
    IfxCan_Can_initMessage(&g_can.txMsg);
    g_can.txMsg.messageId = canMessageID;

    // Ensure that the size of data does not exceed 4 bytes (32 bits)
    if (low_word_size > 4 || high_word_size > 4) {
        // Handle error: data size exceeds 4 bytes
        return;
    }

    // Initialize low_word and high_word to 0
    uint32_t low_word = 0;
    uint32_t high_word = 0;

    // Copy the data to the low_word and high_word
    memcpy(&low_word, low_word_data, low_word_size);
    memcpy(&high_word, high_word_data, high_word_size);

    g_can.txData[0] = low_word; /*To transmit data
    g_can.txData[1] = high_word;


    /*Sends CAN Message, only if BUS is empty
    while( IfxCan_Status_notSentBusy ==
           IfxCan_Can_sendMessage(&g_can.canSrcNode, &g_can.txMsg, &g_can.txData[0]))
    {
    }
}
*/
