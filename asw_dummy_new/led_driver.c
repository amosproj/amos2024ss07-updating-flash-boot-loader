// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Paul Roy <paul.roy@fau.de>
// SPDX-FileCopyrightText: 2024 Leon Wilms <leonwilms.wk@gmail.com>

//============================================================================
// Name        : led_driver.c
// Author      : Paul Roy
// Version     : 0.2
// Copyright   : MIT
// Description : platform-independent header file for LED-Driver
//============================================================================

#include "led_driver.h"
#include "led_driver_TC375_LK.h"

static LED leds[NUM_LEDS];
static int initialized = 0;

/* This function initializes the 2 usable LEDs so they can be turned on and off */
void ledInitDriver(void) 
{
    if (initialized) 
    {
        return;
    }
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i].port = LED_PORTS[i];
        leds[i].pinIndex = LED_PINS[i];
        IfxPort_setPinModeOutput(leds[i].port, leds[i].pinIndex, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
        IfxPort_setPinHigh(leds[i].port, leds[i].pinIndex);
    }
    initialized = 1;
}

/* This function toggles the activity of one specific LED, described by the combination of port and pinIndex.
 * LED on -> LED off
 * LED off -> LED on */
void ledToggleActivity(uint8_t ledNum) 
{
    if (ledNum >= NUM_LEDS) {
        return;
    }
    IfxPort_togglePin(leds[ledNum].port, leds[ledNum].pinIndex);
}

void ledOff(uint8_t ledNum)
{
    if (ledNum >= NUM_LEDS) {
        return;
    }
    IfxPort_setPinHigh(leds[ledNum].port, leds[ledNum].pinIndex);
}

void ledOn(uint8_t ledNum)
{
    if (ledNum >= NUM_LEDS) {
        return;
    }
    IfxPort_setPinLow(leds[ledNum].port, leds[ledNum].pinIndex);
}
