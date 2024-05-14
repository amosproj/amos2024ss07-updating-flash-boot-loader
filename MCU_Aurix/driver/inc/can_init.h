// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Sebastian Rodriguez <r99@melao.de>

//============================================================================
// Name        : can_driver.h
// Author      : Sebastian Rodriguez
// Version     : 0.1
// Copyright   : MIT
// Description : Header for CAN Driver with AURIX Includes
//============================================================================

#include "Ifx_types.h"
#include "IfxCan_Can.h"
#include "IfxCan.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"
#include <stdint.h>

/*********/
/**MACROS*/
/*********/
#define DEBUGGING                   0 //Debug Prints
#define MAXIMUM_CAN_DATA_PAYLOAD    2 /*8Byte CAN-MESSAGE*/
#define INTERRUPT_PRIO_RX           1 /*Priority for RX Interrupt*/
#define INTERRUPT_PRIO_TX           2 /*Prio for TX Interrupt*/
#define CAN_TX_PIN                  &IfxCan_TXD00_P20_8_OUT /*From User Manual 2.5*/
#define CAN_RX_PIN                  &IfxCan_RXD00B_P20_7_IN /*From User Manual 2.5*/
#define CAN_STB                     &MODULE_P20, 6   
/*canType struct contains Data Structures needed for config and processing of CAN Messages*/
typedef struct canType
{
    IfxCan_Can_Config canConfig;                            /* CAN module configuration structure                   */
    IfxCan_Can canModule;                                   /* CAN module handle                                    */
    IfxCan_Can_Node canSrcNode;                             /* CAN source node handle data structure                */
    IfxCan_Can_Node canDstNode;                             /* CAN destination node handle data structure           */
    IfxCan_Can_Node canTXandRXNode;                          /*Handle Data struct for TX and RX*/
    IfxCan_Can_NodeConfig canNodeConfig;                    /* CAN node configuration structure                     */
    IfxCan_Filter canFilter;                                /* CAN filter configuration structure                   */
    IfxCan_Message txMsg;                                   /* Transmitted CAN message structure                    */
    IfxCan_Message rxMsg;                                   /* Received CAN message structure                       */
    uint32_t txData[MAXIMUM_CAN_DATA_PAYLOAD];                /* Transmitted CAN data array                           */
    uint32_t rxData[MAXIMUM_CAN_DATA_PAYLOAD];                /* Received CAN data array                              */
}canType;

typedef struct msg
{
    IfxCan_Message Msg;
    uint32 Data[MAXIMUM_CAN_DATA_PAYLOAD];
}msg;

void canInitDriver(void);
