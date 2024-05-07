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




void (*processDataFunction)(void*);

//TODO: Implement the processDataFunction we want to use


canType g_can; //Global control struct
IfxPort_Pin_Config          g_led1;                         /* Global LED1 configuration and control structure      */
IfxPort_Pin_Config          g_led2;


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
            IfxPort_setPinLow(g_led1.port, g_led1.pinIndex);
        }
        //LED 2 if data TX and RX is the same
        if (g_can.rxData[0] == g_can.txData[0])
        {
            //IfxPort_setPinLow(g_led2.port, g_led2.pinIndex);
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

/**
 * Transmits a CAN Message: Initialize new TX message, TX is transmitted
 * @param canMessageID ID of CAN Message for Prio in BUS
 * @param data data of CAN Message
 * @param len of CAN Message
*/
void canTransmitMessage(uint32_t canMessageID, uint64_t data, uint64_t len){
    IfxCan_Can_initMessage(&g_can.txMsg);
    g_can.txData[0] = data; /*To transmit data*/
    g_can.txMsg.messageId = canMessageID;

    /*Sends CAN Message, only if BUS is empty*/
    while( IfxCan_Status_notSentBusy ==
           IfxCan_Can_sendMessage(&g_can.canSrcNode, &g_can.txMsg, &g_can.txData[0]))
    {
    }
}

void initLeds(void)
{
    /* ======================================================================
     * Configuration of the pins connected to the LEDs:
     * ======================================================================
     *  - define the GPIO port
     *  - define the GPIO pin that is connected to the LED
     *  - define the general GPIO pin usage (no alternate function used)
     *  - define the pad driver strength
     * ======================================================================
     */
    g_led1.port      = &MODULE_P00;
    g_led1.pinIndex  = PIN5;
    g_led1.mode      = IfxPort_OutputIdx_general;
    g_led1.padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    g_led2.port      = &MODULE_P00;
    g_led2.pinIndex  = PIN6;
    g_led2.mode      = IfxPort_OutputIdx_general;
    g_led2.padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Initialize the pins connected to LEDs to level "HIGH", which keep the LEDs turned off as default state */
    IfxPort_setPinHigh(g_led1.port, g_led1.pinIndex);
    IfxPort_setPinHigh(g_led2.port, g_led2.pinIndex);

    /* Set the pin input/output mode for both pins connected to the LEDs */
    IfxPort_setPinModeOutput(g_led1.port, g_led1.pinIndex, IfxPort_OutputMode_pushPull, g_led1.mode);
    IfxPort_setPinModeOutput(g_led2.port, g_led2.pinIndex, IfxPort_OutputMode_pushPull, g_led2.mode);

    /* Set the pad driver mode for both pins connected to the LEDs */
    IfxPort_setPinPadDriver(g_led1.port, g_led1.pinIndex, g_led1.padDriver);
    IfxPort_setPinPadDriver(g_led2.port, g_led2.pinIndex, g_led2.padDriver);
}


