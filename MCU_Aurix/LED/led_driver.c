//============================================================================
// Name        : led_driver.c
// Author      : Paul Roy
// Version     : 0.1
// Copyright   : MIT
// Description : platform-independent header file for LED-Driver
//============================================================================

#include "led_driver.h"

void init_led_driver(void) 
{
    IfxPort_setPinModeOutput(LED1, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinHigh(LED1);
    led1_mode = CONTINUOUS;

    IfxPort_setPinModeOutput(LED2, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinHigh(LED2);
    led2_mode = CONTINUOUS;
}

void set_led_mode(Ifx_P *port, uint8 pinIndex, LED_MODE mode) 
{
    if (pinIndex == 5) 
    {
        led1_mode = mode;
    } 
    else 
    {
        led2_mode = mode;
    }
}

void toggle_led_activity(Ifx_P *port, uint8 pinIndex) 
{
    if ((pinIndex == 5 && led1_mode == CONTINUOUS) || (pinIndex == 6 && led2_mode == CONTINUOUS)) 
    {
        IfxPort_togglePin(*port, pinIndex);
    }
    else 
    {
        
    }
}