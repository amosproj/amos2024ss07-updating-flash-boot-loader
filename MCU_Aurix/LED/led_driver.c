//============================================================================
// Name        : led_driver.c
// Author      : Paul Roy
// Version     : 0.1
// Copyright   : MIT
// Description : platform-independent header file for LED-Driver
//============================================================================

#include "led_driver.h"

/* This function initializes the 2 usable LEDs so they can be turned on and off */
void init_led_driver(void) 
{
    /* Initializing LED1 */
    IfxPort_setPinModeOutput(LED1, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinHigh(LED1);

    /* Initializing LED2 */
    IfxPort_setPinModeOutput(LED2, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinHigh(LED2);
}

/* This function toggles the activity of one specific LED, described by the combination of port and pinIndex.
 * LED on -> LED off
 * LED off -> LED on */
void toggle_led_activity(Ifx_P *port, uint8 pinIndex) 
{
    /* Function call to toggle LED activity */
    IfxPort_togglePin(port, pinIndex);
}

void led_off(Ifx_P *port, uint8 pinIndex)
{
    IfxPort_setPinLow(port, pinIndex);
}

void led_on(Ifx_P *port, uint8 pinIndex)
{
    IfxPort_setPinLow(port, pinIndex);
}