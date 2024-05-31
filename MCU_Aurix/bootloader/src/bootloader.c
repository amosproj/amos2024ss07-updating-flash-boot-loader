// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : loader.c
// Author      : Dorothea Ehrl, Michael Bauer
// Version     : 0.2
// Copyright   : MIT
// Description : Loader initial file
//============================================================================

#include "bootloader.h"

#include "can_driver.h"
#include "can_init.h"
#include "flash.h"
#include "led_driver.h"
#include "isotp.h"
#include "uds.h"
#include "session_manager.h"
#include "memory.h"

uint8_t* rx_uds_message;
uint32_t rx_total_length;

/**
 * @brief: Function to init the bootloader logic
 */
void init_bootloader(void){

    // Init the LED
    init_led_driver();
    led_off(LED1);
    led_off(LED2);

    // Memory
    init_memory();

    // Session Manager
    init_session_manager();

    // Init UDS and CAN
    rx_total_length = 0;
    uds_init();

}

/**
 * @brief: Function to process the cyclic tasks
 */
void cyclicProcessing (void){
    // UDS RX Handling
    rx_uds_message = isotp_rcv(&rx_total_length);
    if(rx_total_length != 0){
        uds_handleRX(rx_uds_message, rx_total_length);
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
    init_led_driver();

    size_t data_size = 64;
    uint32 data[data_size];
    for(size_t i = 0; i < data_size; i++)
    {
        data[i] = i;
    }

    int ret_p = writeProgramFlash(PROGRAM_FLASH_0, PROGRAM_FLASH_0_BASE_ADDR, data, data_size);

    uint32 errors_p = verifyProgramFlash(PROGRAM_FLASH_0_BASE_ADDR, data, data_size);
    if(errors_p == 0 && ret_p == 0)
    {
        led_on(LED2);
    }

    int ret_d = writeDataFlash(DATA_FLASH_0, DATA_FLASH_0_BASE_ADDR, data, data_size);

    uint32 errors_d = verifyDataFlash(DATA_FLASH_0_BASE_ADDR, data, data_size);
    if(errors_d == 0 && ret_d == 0)
    {
        led_on(LED1);
    }
}

