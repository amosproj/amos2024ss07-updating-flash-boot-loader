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
#include "led_driver_TC375_LK.h"

void (*processDataFunction)(void*);

//TODO: Implement the processDataFunction we want to use
rx_ringbuffer rx_buffer = { {0}, 0, 0 };


/**
 * Write data in buffer
 * Takes uint32_t data and writes it in uint8_t buffer taking byte by byte
*/
void write_rx_buffer_32(uin32_t* data, size_t size){
    for(size_t i = 0; i < size; i++){
        rx_buffer.Data[rx_buffer.next_write] = (uint8_t)(*data >> (8*i)) & 0xFF;
        rx_buffer.next_write = (rx_buffer.next_write + 1) % RX_BUFFER_SIZE;
    }
}


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
    
     //Read Message
    if (processDataFunction != NULL)
    {
        //Callback
    }
    
        IfxCan_Node_clearInterruptFlag(g_can.canTXandRXNode.node, IfxCan_Interrupt_rxFifo0NewMessage); /*Clear Message Stored Flag*/
        IfxCan_Can_readMessage(&g_can.canTXandRXNode, &g_can.rxMsg, (uint32*)g_can.rxData);
        writeInBuffer(g_can.rxData, 8);

}

 
void canAcceptAllMessagesFilter(void){
    g_can.canFilter.number = 0;
    g_can.canFilter.elementConfiguration = IfxCan_FilterElementConfiguration_storeInRxFifo0;
    g_can.canFilter.type = IfxCan_FilterType_classic;
    g_can.canFilter.id1 = 0x0FF;
    g_can.canFilter.id2 = 0x700;
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
    g_can.canNodeConfig.filterConfig.extendedFilterForNonMatchingFrames = IfxCan_NonMatchingFrame_reject;
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
rx_buffer* canInitDriver(void){
    IfxCan_Can_initModuleConfig(&g_can.canConfig, &MODULE_CAN0); /*LoadsDefault Config*/
    IfxCan_Can_initModule(&g_can.canModule, &g_can.canConfig); /*Init with default config*/

    initTXandRXNode();
    // canAcceptAllMessagesFilter();

    
    IfxCan_Can_initMessage(&g_can.rxMsg); /*Init for RX Message*/
    return &rx_buffer;g
}

/**
 * Transmits a CAN Message: Initialize new TX message, TX is transmitted
 * @param canMessageID ID of CAN Message for Prio in BUS
 * @param data data of CAN Message
 * @param len of CAN Message
*/
void canTransmitMessage(uint32_t canMessageID, uint32_t lowWord, uint32_t highWord){
    IfxCan_Can_initMessage(&g_can.txMsg);
    g_can.txData[0] = lowWord; /*To transmit data*/
    g_can.txData[1] = highWord;
    g_can.txMsg.messageId = canMessageID;

    /*Sends CAN Message, only if BUS is empty*/
        while( IfxCan_Status_notSentBusy ==
           IfxCan_Can_sendMessage(&g_can.canTXandRXNode, &g_can.txMsg, (uint32*)&g_can.txData[0])){}

}

void canDummyMessagePeriodicly(void){
    canTransmitMessage(0x123, 0x12345678, 0x87654321);
    waitTime(IfxStm_getTicksFromMilliseconds(BSP_DEFAULT_TIMER, 500)); 
}
