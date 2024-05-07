// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>

//============================================================================
// Name        : flash.c
// Author      : Dorothea Ehrl
// Version     : 0.1
// Copyright   : MIT
// Description : Flash wrapper for Bootloader
//============================================================================

#include <string.h>

#include "flash/flash.h"
#include "IfxFlash.h"
#include "IfxCpu.h"


/* Definition of the LEDs port pins */
#define LED1                        &MODULE_P00,5               /* LED connected to Port 00, Pin 5                  */
#define LED2                        &MODULE_P00,6               /* LED connected to Port 00, Pin 6                  */

void set_led(void)
{
    /* Configure LED1 and LED2 port pins */
    IfxPort_setPinMode(LED1, IfxPort_Mode_outputPushPullGeneral);
    IfxPort_setPinMode(LED2, IfxPort_Mode_outputPushPullGeneral);

    // turn on led2
    IfxPort_setPinState(LED2, IfxPort_State_low);

    IfxPort_setPinState(LED1, IfxPort_State_high);
}
