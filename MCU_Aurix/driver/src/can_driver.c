// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Sebastian Rodriguez <r99@melao.de>

//============================================================================
// Name        : can_driver.c
// Author      : Sebastian Rodriguez, Leon Wilms
// Version     : 0.2
// Copyright   : MIT
// Description : C File for CAN Driver
//============================================================================

#include "can_driver.h"
#include "can_driver_TC375_LK.h"
#include "led_driver.h"
#include "led_driver_TC375_LK.h"

void (*processDataFunction)(uint32_t*, IfxCan_DataLengthCode);

//TODO: Implement the processDataFunction we want to use


canType g_can; //Global control struct

IfxCan_Can_Pins canPins = {
    .padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed2,
    .rxPin = CAN_RX_PIN,
    .rxPinMode = IfxPort_InputMode_noPullDevice,
    .txPin = CAN_TX_PIN,
    .txPinMode = IfxPort_OutputMode_pushPull
};

/*Interrupts*/
IFX_INTERRUPT(canIsrTxHandler, 0, INTERRUPT_PRIO_TX);
IFX_INTERRUPT(canIsrRxFifo0Handler, 0, INTERRUPT_PRIO_RX);

void canIsrTxHandler(void){
      IfxCan_Node_clearInterruptFlag(g_can.canTXandRXNode.node, IfxCan_Interrupt_transmissionCompleted);
}

/**
 * Interrupt Service Routine (ISR) is called when RX Interrupt is generated
 * Calls function to execute on Data Read in CAN Message
 * @param processDataFunction Pointer to function that processes Data read in CAN Message
*/
void canIsrRxFifo0Handler(){
        IfxCan_Node_clearInterruptFlag(g_can.canTXandRXNode.node, IfxCan_Interrupt_rxFifo0NewMessage); /*Clear Message Stored Flag*/
        IfxCan_Can_readMessage(&g_can.canTXandRXNode, &g_can.rxMsg, (uint32*)g_can.rxData);

        // TODO: Filter for Own ECU ID or Broadcast messages - Do not call processDataFunction if message is for other ECU
        processDataFunction(g_can.rxData, g_can.rxMsg.dataLengthCode); //has to be casted in ISO-Tp

}

 
void canAcceptAllMessagesFilter(void){
    g_can.canFilter.number = 0;
    g_can.canFilter.elementConfiguration = IfxCan_FilterElementConfiguration_storeInRxFifo0;
    g_can.canFilter.type = IfxCan_FilterType_classic;

    // Testing lines
    g_can.canFilter.id1 = 0x0F24; // TODO: Change based on Memory DID
    g_can.canFilter.id2 = 0xFFFFFFFF;

    //TESTING

    //g_can.canFilter.rxBufferOffset = IfxCan_RxBufferId_0;
    IfxCan_Can_setStandardFilter(&g_can.canTXandRXNode, &g_can.canFilter);
}

void initTXandRXNode(void){
    IfxCan_Can_initNodeConfig(&g_can.canNodeConfig, &g_can.canModule);                  /*Default Config*/

    g_can.canNodeConfig.busLoopbackEnabled = FALSE;                                      /*Loopbackmode*/
    g_can.canNodeConfig.nodeId = IfxCan_NodeId_0;                                         /*Node ID 0 -> is must*/
    
    /*FRAME TYPE RX AND TX*/
    g_can.canNodeConfig.frame.type = IfxCan_FrameType_transmitAndReceive;
    g_can.canNodeConfig.rxConfig.rxFifo0DataFieldSize = IfxCan_DataFieldSize_64;
    g_can.canNodeConfig.rxConfig.rxFifo0Size = 15;
    g_can.canNodeConfig.rxConfig.rxMode = IfxCan_RxMode_fifo0;

    /*PIN Definition*/
    g_can.canNodeConfig.pins = &canPins;

    /*Filter config*/
    g_can.canNodeConfig.filterConfig.messageIdLength = IfxCan_MessageIdLength_both;
    g_can.canNodeConfig.filterConfig.standardListSize = 0;
    g_can.canNodeConfig.filterConfig.extendedListSize = 0;
    g_can.canNodeConfig.filterConfig.standardFilterForNonMatchingFrames = IfxCan_NonMatchingFrame_acceptToRxFifo0;
    g_can.canNodeConfig.filterConfig.extendedFilterForNonMatchingFrames = IfxCan_NonMatchingFrame_acceptToRxFifo0;
    g_can.canNodeConfig.filterConfig.rejectRemoteFramesWithStandardId = TRUE;
    g_can.canNodeConfig.filterConfig.rejectRemoteFramesWithExtendedId = TRUE;

    /*Interrupt Config*/
    g_can.canNodeConfig.interruptConfig.rxFifo0NewMessageEnabled = TRUE;
    g_can.canNodeConfig.interruptConfig.rxf0n.priority = INTERRUPT_PRIO_RX;           /*Prio*/
    g_can.canNodeConfig.interruptConfig.rxf0n.interruptLine = IfxCan_InterruptLine_1;   /*Interrupt Line 1*/
    g_can.canNodeConfig.interruptConfig.rxf0n.typeOfService = IfxSrc_Tos_cpu0;          /*On CPU 0*/

    g_can.canNodeConfig.interruptConfig.transmissionCompletedEnabled = TRUE;     /*Raises Interrupt when transmition is done*/
    g_can.canNodeConfig.interruptConfig.traco.priority = INTERRUPT_PRIO_TX;    /*Prio*/
    g_can.canNodeConfig.interruptConfig.traco.interruptLine = IfxCan_InterruptLine_0; /*Interrupt line 0*/
    g_can.canNodeConfig.interruptConfig.traco.typeOfService = IfxSrc_Tos_cpu0;       /*On CPU0*/
    IfxPort_setPinModeOutput(CAN_STB, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(CAN_STB);
    IfxCan_Can_initNode(&g_can.canTXandRXNode, &g_can.canNodeConfig); //Init Node with CAN Pin Config and Standard Baud Rate 500k
    
}

/**
 * Initialize CAN Module and Node
*/
void canInitDriver(void (*processData)(uint32_t*, IfxCan_DataLengthCode)){
    IfxCan_Can_initModuleConfig(&g_can.canConfig, &MODULE_CAN0); /*LoadsDefault Config*/
    IfxCan_Can_initModule(&g_can.canModule, &g_can.canConfig); /*Init with default config*/

    initTXandRXNode();
    canAcceptAllMessagesFilter();
    
    IfxCan_Can_initMessage(&g_can.rxMsg); /*Init for RX Message*/
    g_can.rxMsg.readFromRxFifo0 = TRUE; /*Read from FIFO0*/

    processDataFunction = processData;
}

/**
 * Transmits a CAN Message: Initialize new TX message, TX is transmitted
 * @param canMessageID, ID of CAN Message for Prio in BUS
 * @param data,  data of CAN Message
 * @param size, len of CAN Message
*/
int canTransmitMessage(uint32_t canMessageID, uint8_t* data, size_t size){
    toggle_led_activity(LED2);
    IfxCan_Can_initMessage(&g_can.txMsg);
    g_can.txMsg.messageId = canMessageID;
    g_can.txMsg.messageIdLength = IfxCan_MessageIdLength_extended;

    //Not sure if necessary ~Leon
    // Ensure that the size of data does not exceed 8 bytes (32 bits)
    if (size > 8) {
        // Handle error: data size exceeds 4 bytes
        return -1;
    }

    // Initialize g_can.txData to zero
    g_can.txData[0] = 0;
    g_can.txData[1] = 0;

    g_can.txMsg.dataLengthCode = size;

    // Copy up to 8 bytes of data into g_can.txData
    memcpy(g_can.txData, data, size);


    //Sends CAN Message, only if BUS is empty
    while( IfxCan_Status_notSentBusy ==
           IfxCan_Can_sendMessage(&g_can.canTXandRXNode, &g_can.txMsg, &g_can.txData[0]))
    {

    }
    return 0;
}
