// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Paul Roy <paul.roy@fau.de>
// SPDX-FileCopyrightText: 2024 Leon Wilms <leonwilms.wk@gmail.com>

//============================================================================
// Name        : led_driver.h
// Author      : Paul Roy
// Version     : 0.
// Copyright   : MIT
// Description : platform-independent header file for LED-Driver
//============================================================================

#ifndef LED_DRIVER_H_
#define LED_DRIVER_H_

/******************************************************************************/
/*---------------------------------Includes-----------------------------------*/
/******************************************************************************/

#include <stdint.h>

/******************************************************************************/
/*----------------------------Function Prototypes-----------------------------*/
/******************************************************************************/

void ledInitDriver(void);
void ledToggleActivity(uint8_t ledNum);

void ledOff(uint8_t ledNum);
void ledOn(uint8_t ledNum);

#endif
