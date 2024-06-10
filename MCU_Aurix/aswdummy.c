// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Sebastian Rodriguez <r99@melao.de>

//============================================================================
// Name        : aswdummy.h
// Author      : Sebastian Rodriguez
// Version     : 0.1
// Copyright   : MIT
// Description : Implementation file for the dummy ASW
//============================================================================

#include "aswdummy.h"
#include "led_driver.h"

version_info global_info = {
    .magic_number = 0xA305FB12024FB124,
    .asw_version = "v0.34",
    .asw_version_comment = "Bremse vorne links",
    .magic_number2 = 0xA305FB12024FB124,
};

void alternating_blinking(void){
    ledToggleActivity(leds[0]);
    ledToggleActivity(leds[1]);
}