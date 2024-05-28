// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>

//============================================================================
// Name        : loader.c
// Author      : Dorothea Ehrl
// Version     : 0.1
// Copyright   : MIT
// Description : Loader initial file
//============================================================================

#include "bootloader.h"

#include "can_driver.h"
#include "can_init.h"
#include "flash.h"
#include "led_driver.h"
#include "uds.h"

void show_led(void)
{
    toggle_led_activity(LED1);
    toggle_led_activity(LED1);

    led_on(LED2);
    led_off(LED2);
}

void show_can(void)
{
    init_led_driver();

    void (*processData)(void*); // TODO correct function
    canInitDriver(processData);

    /*
     * ------------------------------------------------------------------------
     * TESTING
     * ------------------------------------------------------------------------
     */
    //ISOTP: Edited canTransmit to test new isoTP implementation
    //uint8_t debugMessage = CAN_DEBUG_DATA2;
    //canTransmitMessage(CAN_DEBUG_ID, &debugMessage, 1);

    canTransmitMessage(CAN_DEBUG_ID,CAN_DEBUG_DATA, 1);


    /*
     * ------------------------------------------------------------------------
     * TESTING
     * ------------------------------------------------------------------------
     */
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

void show_uds_rx_read_data(void)
{
    int len;
    uint8* data = _create_read_data_by_ident(&len, 0, FBL_DID_SYSTEM_NAME, 0, 0);
    uds_handleRX(data, len);
}
