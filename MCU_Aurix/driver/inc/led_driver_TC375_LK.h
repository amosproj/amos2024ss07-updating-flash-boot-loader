// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Paul Roy <paul.roy@fau.de>
// SPDX-FileCopyrightText: 2024 Leon Wilms <leonwilms.wk@gmail.com>

//============================================================================
// Name        : led_driver_TC375_LK.h
// Author      : Paul Roy
// Version     : 0.1
// Copyright   : MIT
// Description : LED-Driver header file for TC375 LK
//============================================================================

#ifndef LED_DRIVER_TC375_LK_H_
#define LED_DRIVER_TC375_LK_H_

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/

#define LED1    &MODULE_P00,5
#define LED2    &MODULE_P00,6

/******************************************************************************/
/*----------------------------Function Prototypes-----------------------------*/
/******************************************************************************/

void init_led_driver(void);

#endif
