/**********************************************************************************************************************
 * \file Cpu0_Main.c
 * \copyright Copyright (C) Infineon Technologies AG 2019
 * 
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of 
 * business you are acting and (ii) Infineon Technologies AG or its licensees. If and as long as no such terms of use
 * are agreed, use of this file is subject to following:
 * 
 * Boost Software License - Version 1.0 - August 17th, 2003
 * 
 * Permission is hereby granted, free of charge, to any person or organization obtaining a copy of the software and 
 * accompanying documentation covered by this license (the "Software") to use, reproduce, display, distribute, execute,
 * and transmit the Software, and to prepare derivative works of the Software, and to permit third-parties to whom the
 * Software is furnished to do so, all subject to the following:
 * 
 * The copyright notices in the Software and this entire statement, including the above license grant, this restriction
 * and the following disclaimer, must be included in all copies of the Software, in whole or in part, and all 
 * derivative works of the Software, unless such copies or derivative works are solely in the form of 
 * machine-executable object code generated by a source language processor.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE 
 * COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN 
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 *********************************************************************************************************************/

#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"

#include "loader.h"
#include "led_driver.h"
#include "can_driver.h"
#include "can_init.h"

/*
 * ------------------------------------------------------------------------
 * TESTING
 * ------------------------------------------------------------------------
 */

#include <stdio.h>
#include <uds_comm_spec.h>
#include "Bsp.h"

//IsoTpContext ctx;


/*
 * ------------------------------------------------------------------------
 * TESTING
 * ------------------------------------------------------------------------
 */


IFX_ALIGN(4) IfxCpu_syncEvent g_cpuSyncEvent = 0;

void core0_main(void)
{
    IfxCpu_enableInterrupts();
    
    /* !!WATCHDOG0 AND SAFETY WATCHDOG ARE DISABLED HERE!!
     * Enable the watchdogs and service them periodically if it is required
     */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());
    
    /* Wait for CPU sync event */
    IfxCpu_emitEvent(&g_cpuSyncEvent);
    IfxCpu_waitEvent(&g_cpuSyncEvent, 1);


    //SerialASC.begin(9600);

    init_led_driver();
    //show_flash();

    canInitDriver();

    led_off(LED1);
    led_off(LED2);

    //isotp_init(&ctx);

    uint8_t dataCAN[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

    uint8_t dataIsoSolo[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    uint8_t dataIsoMulti[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};



    uint32_t data_in_len = sizeof(dataIsoMulti);
    uint32_t data_out_idx_ctr = 0;
    uint8_t frame_idx = 0;
    int data_out_len;
    int has_next;
    uint8_t max_len_per_frame = 8;

    while(1)
    {
        waitTime(IfxStm_getTicksFromMilliseconds(BSP_DEFAULT_TIMER, 1000));

        toggle_led_activity(LED1);

        //isotp_poll(&ctx);

        //SerialASC.print("Test\n");

        //printf("TESTING\n");

        // Create and send the first frame
        uint8_t* first_frame = tx_starting_frame(&data_out_len,
                                                    &has_next,
                                                    max_len_per_frame,
                                                    dataIsoMulti,
                                                    data_in_len,
                                                    &data_out_idx_ctr);

        canTransmitMessage(0x123, first_frame, data_out_len);
        free(first_frame);

        // Send consecutive frames if necessary
        while (has_next) {
            uint8_t* consecutive_frame = tx_consecutive_frame(&data_out_len,
                                                                &has_next,
                                                                max_len_per_frame,
                                                                dataIsoMulti,
                                                                data_in_len,
                                                                &data_out_idx_ctr,
                                                                &frame_idx);

            canTransmitMessage(0x123, consecutive_frame, data_out_len);
            free(consecutive_frame);
        }



        //canTransmitMessage(0x123, dataCAN, sizeof(dataCAN));


    }
}
