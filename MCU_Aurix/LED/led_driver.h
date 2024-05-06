//============================================================================
// Name        : led_driver.h
// Author      : Paul Roy
// Version     : 0.1
// Copyright   : MIT
// Description : platform-independent header file for LED-Driver
//============================================================================

#ifndef LED_DRIVER_H_
#define LED_DRIVER_H_

#include "IfxPort.h"

#include "led_driver_TC375_LK.h"

#define LED_MODE        uint8
#define BLINK           1
#define CONTINUOUS      2

void set_led_mode(Ifx_P *, uint8, LED_MODE);
void toggle_led_activity(Ifx_P *, uint8);

#endif
