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
/*----------------------------Function Prototypes-----------------------------*/
/******************************************************************************/

void ledToggleActivity(uint8 pinIndex);

void ledOff(uint8 pinIndex);
void ledOn(uint8 pinIndex);

#endif
