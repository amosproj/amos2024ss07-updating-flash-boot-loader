// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : loader.c
// Author      : Dorothea Ehrl, Michael Bauer
// Version     : 0.3
// Copyright   : MIT
// Description : Loader initial file
//============================================================================

#include <stdlib.h>

#include "bootloader.h"

#include "can_driver.h"
#include "can_driver_TC375_LK.h"
#include "flash_driver.h"
#include "flash_driver_TC375_LK.h"
#include "led_driver.h"
#include "led_driver_TC375_LK.h"
#include "isotp.h"
#include "uds.h"
#include "session_manager.h"
#include "memory.h"
#include "flashing.h"

uint8_t* rx_uds_message_single;
uint32_t rx_total_length_single;

uint8_t* rx_uds_message_multi;
uint32_t rx_total_length_multi;

/**
 * @brief: Function to init the bootloader logic
 */
void init_bootloader(void){

    // Init the LED
    ledInitDriver();
    ledOff(0);
    ledOff(0);

    // Memory
    init_memory();

    // Flashing
    flashingInit();

    // Session Manager
    init_session_manager();

    // Init UDS and CAN
    rx_total_length_single = 0;
    rx_total_length_multi = 0;
    uds_init();

}

/**
 * @brief: Function to process the cyclic tasks
 */
void cyclicProcessing (void){
    // UDS RX Handling

    rx_uds_message_single = isotp_single_rcv(&rx_total_length_single);
    if(rx_total_length_single != 0){
        ledToggleActivity(0);
        // New RX message is created using calloc
        uds_handleRX(rx_uds_message_single, rx_total_length_single);
        free(rx_uds_message_single);
    }

    rx_uds_message_multi = isotp_multi_rcv(&rx_total_length_multi);
    if(rx_total_length_multi != 0){
        ledToggleActivity(0);
        // RX Buffer of Multiframe is used, no need to free the buffer
        uds_handleRX(rx_uds_message_multi, rx_total_length_multi);
        rx_reset_isotp_multi_buffer();
    }
}

/**
 * @brief: Function to deinit the bootloader logic
 */
void deinit_bootloader(void){
    uds_close();
}

void show_flash(void)
{
    ledInitDriver();

    size_t data_size = 64;
    uint32 data[data_size];
    for(size_t i = 0; i < data_size; i++)
    {
        data[i] = i;
    }

    bool ret_p = flashWrite(PROGRAM_FLASH_0_BASE_ADDR, data, data_size);

    bool errors_p = flashVerify(PROGRAM_FLASH_0_BASE_ADDR, data, data_size);
    if(errors_p && ret_p)
    {
        ledOn(1);
    }

    bool ret_d = flashWrite(DATA_FLASH_0_BASE_ADDR, data, data_size);

    bool errors_d = flashVerify(DATA_FLASH_0_BASE_ADDR, data, data_size);
    if(errors_d && ret_d)
    {
        ledOn(0);
    }
}

