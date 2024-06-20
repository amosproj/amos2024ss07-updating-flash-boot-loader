// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>
// SPDX-FileCopyrightText: 2024 Sebastian Rodriguez <r99@melao.de>

//============================================================================
// Name        : loader.c
// Author      : Dorothea Ehrl, Michael Bauer, Sebastian Rodriguez
// Version     : 0.3
// Copyright   : MIT
// Description : Loader initial file
//============================================================================

#include <time.h>

#include "bootloader.h"

#include "Ifx_Ssw_CompilersTasking.h" //For the bootloaderJumpToApp function
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

uint8_t* rx_uds_message;
uint32_t rx_total_length;

time_t seconds = time(NULL);

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

    // Session Manager
    init_session_manager();

    // Init UDS and CAN
    rx_total_length = 0;
    uds_init();

}

/**
 * @brief: Function to jump to the application software
 */
void bootloaderJumpToASW(void){
    //TODO: 
    uds_close();
    unsigned int TRAPTABASW = 0x80090100;
    unsigned int INTTABASW = 0x804F4000;
    unsigned int ISTACKASW;
    unsigned int USTACKASW;


    //TODO: Reset Peripherals, Lock Flash again -> schon durch flash driver
    //Can, gpio + clock
    //Not sure how to do it

    //Disable Interrupts
    IfxCpu_disableInterrupts();
    //Clear Interrupt flags
    //TODO: Should be cleared by can_driver no other flags are used ? or should we really clear all can flags again?
    
    //Set Vector Table offset
    Ifx_Ssw_MTCR(CPU_BTV, (unsigned int)__TRAPTABASW); //Base Trap Vector Table
    Ifx_Ssw_MTCR(CPU_BIV, (unsigned int)__INTTABASW); //Base Interrupt Vector Table

    //Set Stack Pointer
    Ifx_Ssw_MTCR(CPU_ISP, (unsigned int)__ISTACKASW); //Interrupt Stack Pointer
    Ifx_Ssw_setAddressReg(a10, __USTACKASW); //User Stack Pointer
    //magic numbers im arbeitspeicher -> jump zum bootloader oder nicht -> App reset
    //Jump to Start Address                                            
    Ifx_Ssw_jumpToFunction(ASWStartAddress); //or __asm("ja 0xA0800000"); ?
}

/**
 * @brief: Function to process the cyclic tasks
 */
void cyclicProcessing (void){
    // UDS RX Handling

    rx_uds_message = isotp_rcv(&rx_total_length);
    if(rx_total_length != 0){
        ledToggleActivity(0);
        uds_handleRX(rx_uds_message, rx_total_length);
        time(&seconds); //Assumes no tester present was received
    }
    else if (difftime(time(NULL), seconds) > 5)
    {
        bootloaderJumpToASW();
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

