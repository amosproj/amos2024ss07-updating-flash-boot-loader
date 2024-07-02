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

#include "Ifx_types.h"

void (*processDataFunction)(uint32_t*, IfxCan_DataLengthCode);

//TODO: Implement the processDataFunction we want to use


Can_Type can_g; //Global control struct

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
      IfxCan_Node_clearInterruptFlag(can_g.canTXandRXNode.node, IfxCan_Interrupt_transmissionCompleted);
}

/**
 * Interrupt Service Routine (ISR) is called when RX Interrupt is generated
 * Calls function to execute on Data Read in CAN Message
 * @param processDataFunction Pointer to function that processes Data read in CAN Message
*/
void canIsrRxFifo0Handler(){
        IfxCan_Node_clearInterruptFlag(can_g.canTXandRXNode.node, IfxCan_Interrupt_rxFifo0NewMessage); /*Clear Message Stored Flag*/
        IfxCan_Can_readMessage(&can_g.canTXandRXNode, &can_g.rxMsg, (uint32*)can_g.rxData);

        // TODO: Filter for Own ECU ID or Broadcast messages - Do not call processDataFunction if message is for other ECU
        processDataFunction(can_g.rxData, can_g.rxMsg.dataLengthCode); //has to be casted in ISO-Tp

}

 
static void canAcceptAllMessagesFilter(void){
    can_g.canFilter.number = 0;
    can_g.canFilter.elementConfiguration = IfxCan_FilterElementConfiguration_storeInRxFifo0;
    can_g.canFilter.type = IfxCan_FilterType_classic;

    // Testing lines
    can_g.canFilter.id1 = 0x0F24; // TODO: Change based on Memory DID
    can_g.canFilter.id2 = 0xFFFFFFFF;

    //TESTING

    //g_can.canFilter.rxBufferOffset = IfxCan_RxBufferId_0;
    IfxCan_Can_setStandardFilter(&can_g.canTXandRXNode, &can_g.canFilter);
}

static void canInitTXandRXNode(void){
    IfxCan_Can_initNodeConfig(&can_g.canNodeConfig, &can_g.canModule);                  /*Default Config*/

    can_g.canNodeConfig.busLoopbackEnabled = FALSE;                                      /*Loopbackmode*/
    can_g.canNodeConfig.nodeId = IfxCan_NodeId_0;                                         /*Node ID 0 -> is must*/
    
    /*FRAME TYPE RX AND TX*/
    can_g.canNodeConfig.frame.type = IfxCan_FrameType_transmitAndReceive;
    can_g.canNodeConfig.rxConfig.rxFifo0DataFieldSize = IfxCan_DataFieldSize_64;
    can_g.canNodeConfig.rxConfig.rxFifo0Size = 15;
    can_g.canNodeConfig.rxConfig.rxMode = IfxCan_RxMode_fifo0;

    /*PIN Definition*/
    can_g.canNodeConfig.pins = &canPins;

    /*Filter config*/
    can_g.canNodeConfig.filterConfig.messageIdLength = IfxCan_MessageIdLength_both;
    can_g.canNodeConfig.filterConfig.standardListSize = 0;
    can_g.canNodeConfig.filterConfig.extendedListSize = 0;
    can_g.canNodeConfig.filterConfig.standardFilterForNonMatchingFrames = IfxCan_NonMatchingFrame_acceptToRxFifo0;
    can_g.canNodeConfig.filterConfig.extendedFilterForNonMatchingFrames = IfxCan_NonMatchingFrame_acceptToRxFifo0;
    can_g.canNodeConfig.filterConfig.rejectRemoteFramesWithStandardId = TRUE;
    can_g.canNodeConfig.filterConfig.rejectRemoteFramesWithExtendedId = TRUE;

    /*Interrupt Config*/
    can_g.canNodeConfig.interruptConfig.rxFifo0NewMessageEnabled = TRUE;
    can_g.canNodeConfig.interruptConfig.rxf0n.priority = INTERRUPT_PRIO_RX;           /*Prio*/
    can_g.canNodeConfig.interruptConfig.rxf0n.interruptLine = IfxCan_InterruptLine_1;   /*Interrupt Line 1*/
    can_g.canNodeConfig.interruptConfig.rxf0n.typeOfService = IfxSrc_Tos_cpu0;          /*On CPU 0*/

    can_g.canNodeConfig.interruptConfig.transmissionCompletedEnabled = TRUE;     /*Raises Interrupt when transmition is done*/
    can_g.canNodeConfig.interruptConfig.traco.priority = INTERRUPT_PRIO_TX;    /*Prio*/
    can_g.canNodeConfig.interruptConfig.traco.interruptLine = IfxCan_InterruptLine_0; /*Interrupt line 0*/
    can_g.canNodeConfig.interruptConfig.traco.typeOfService = IfxSrc_Tos_cpu0;       /*On CPU0*/
    IfxPort_setPinModeOutput(CAN_STB, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(CAN_STB);
    IfxCan_Can_initNode(&can_g.canTXandRXNode, &can_g.canNodeConfig); //Init Node with CAN Pin Config and Standard Baud Rate 500k
    
}

/**
 * Initialize CAN Module and Node
*/
void canInitDriver(void (*processData)(uint32_t*, IfxCan_DataLengthCode)){
    IfxCan_Can_initModuleConfig(&can_g.canConfig, &MODULE_CAN0); /*LoadsDefault Config*/
    IfxCan_Can_initModule(&can_g.canModule, &can_g.canConfig); /*Init with default config*/

    canInitTXandRXNode();
    canAcceptAllMessagesFilter();
    
    IfxCan_Can_initMessage(&can_g.rxMsg); /*Init for RX Message*/
    can_g.rxMsg.readFromRxFifo0 = TRUE; /*Read from FIFO0*/

    processDataFunction = processData;
}

/**
 * Transmits a CAN Message: Initialize new TX message, TX is transmitted
 * @param canMessageID, ID of CAN Message for Prio in BUS
 * @param data,  data of CAN Message
 * @param size, len of CAN Message
*/
int canTransmitMessage(uint32_t canMessageID, uint8_t* data, size_t size){
    ledToggleActivity(1);
    IfxCan_Can_initMessage(&can_g.txMsg);
    can_g.txMsg.messageId = canMessageID;
    can_g.txMsg.messageIdLength = IfxCan_MessageIdLength_extended;

    //Not sure if necessary ~Leon
    // Ensure that the size of data does not exceed 8 bytes (32 bits)
    if (size > 8) {
        // Handle error: data size exceeds 4 bytes
        return -1;
    }

    // Initialize g_can.txData to zero
    can_g.txData[0] = 0;
    can_g.txData[1] = 0;

    can_g.txMsg.dataLengthCode = size;

    // Copy up to 8 bytes of data into can_g.txData
    memcpy(can_g.txData, data, size);


    //Sends CAN Message, only if BUS is empty
    while( IfxCan_Status_notSentBusy ==
           IfxCan_Can_sendMessage(&can_g.canTXandRXNode, &can_g.txMsg, (uint32*) &can_g.txData[0]))
    {

    }
    return 0;
}
