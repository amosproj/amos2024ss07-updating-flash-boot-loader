// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Paul Roy <paul.roy@fau.de>
// SPDX-FileCopyrightText: 2024 Leon Wilms <leonwilms.wk@gmail.com>

//============================================================================
// Name        : led_driver.h
// Author      : Paul Roy
// Version     : 0.1
// Copyright   : MIT
// Description : platform-independent header file for LED-Driver
//============================================================================

#ifndef LED_DRIVER_H_
#define LED_DRIVER_H_

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/

#include "IfxPort.h"

#include "led_driver_TC375_LK.h"

/******************************************************************************/
/*----------------------------Function Prototypes-----------------------------*/
/******************************************************************************/

void toggle_led_activity(Ifx_P *, uint8);

void led_off(Ifx_P *port, uint8 pinIndex);
void led_on(Ifx_P *port, uint8 pinIndex);

#endif
