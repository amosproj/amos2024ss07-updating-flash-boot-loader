//============================================================================
// Name        : led_driver.c
// Author      : Paul Roy
// Version     : 0.2
// Copyright   : MIT
// Description : platform-independent header file for LED-Driver
//============================================================================

#include "led_driver.h"
#include "led_driver_TC375_LK.h"

typedef struct
{
    Ifx_P *port;
    uint8 pinIndex;
} LED;

static LED leds[NUM_LEDS];

/* This function initializes the 2 usable LEDs so they can be turned on and off */
void ledInitDriver(void) 
{
    /* Initializing LED-array*/
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i].port = LED_PORTS[i];
        leds[i].pinIndex = LED_PINS[i];
    }

    /* Initializing LEDs */
    for (int i = 0; i < NUM_LEDS; i++) {
        IfxPort_setPinModeOutput(leds[i].port, leds[i].pinIndex, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
        IfxPort_setPinHigh(leds[i].port, leds[i].pinIndex);
    }
}

/* This function toggles the activity of one specific LED, described by the combination of port and pinIndex.
 * LED on -> LED off
 * LED off -> LED on */
void ledToggleActivity(uint8 ledNum) 
{
    if (ledNum >= NUM_LEDS) {
        return;
    }
    IfxPort_togglePin(leds[ledNum].port, leds[ledNum].pinIndex);
}

void ledOff(uint8 ledNum)
{
    if (ledNum >= NUM_LEDS) {
        return;
    }
    IfxPort_setPinHigh(leds[ledNum].port, leds[ledNum].pinIndex);
}

void ledOn(uint8 ledNum)
{
    if (ledNum >= NUM_LEDS) {
        return;
    }
    IfxPort_setPinLow(leds[ledNum].port, leds[ledNum].pinIndex);
}
