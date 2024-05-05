/**
 * \file IfxGtm_Dtm.c
 * \brief GTM  basic functionality
 *
 * \version iLLD_1_0_1_12_0
 * \copyright Copyright (c) 2020 Infineon Technologies AG. All rights reserved.
 *
 *
 *                                 IMPORTANT NOTICE
 *
 * Use of this file is subject to the terms of use agreed between (i) you or
 * the company in which ordinary course of business you are acting and (ii)
 * Infineon Technologies AG or its licensees. If and as long as no such terms
 * of use are agreed, use of this file is subject to following:
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer, must
 * be included in all copies of the Software, in whole or in part, and all
 * derivative works of the Software, unless such copies or derivative works are
 * solely in the form of machine-executable object code generated by a source
 * language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/

#include "IfxGtm_Dtm.h"

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/

Ifx_GTM_CDTM_DTM *IfxGtm_Dtm_getDtmPointer(Ifx_GTM *gtm, IfxGtm_Cdtm cdtmIndex, IfxGtm_Dtm dtmIndex)
{
    Ifx_GTM_CDTM_DTM *dtmSFR;

    if ((cdtmIndex > IfxGtm_Cdtm_none) && (cdtmIndex <= IfxGtm_Cdtm_6))
    {
        dtmSFR = &(gtm->CDTM[cdtmIndex].DTM[dtmIndex]);
    }
    else
    {
        dtmSFR = NULL_PTR;
    }

    return dtmSFR;
}


float32 IfxGtm_Dtm_getClockFrequency(Ifx_GTM *gtm, IfxGtm_Cdtm cdtmIndex, IfxGtm_Dtm dtmIndex)
{
    Ifx_GTM_CDTM_DTM *dtm;
    float32           frequency = 0.0;

    dtm = IfxGtm_Dtm_getDtmPointer(gtm, cdtmIndex, dtmIndex);

    if (dtm != NULL_PTR)
    {
        IfxGtm_Dtm_ClockSource clockSource;
        clockSource = IfxGtm_Dtm_getClockSource(dtm);

        if (dtmIndex <= IfxGtm_Dtm_3)
        {   /* Connected to TOMs */
            switch (clockSource)
            {
            case IfxGtm_Dtm_ClockSource_systemClock:
                frequency = IfxScuCcu_getGtmFrequency();
                break;
            case IfxGtm_Dtm_ClockSource_cmuClock0:
                frequency = IfxGtm_Cmu_getFxClkFrequency(gtm, IfxGtm_Cmu_Clk_0, TRUE);
                break;
            case IfxGtm_Dtm_ClockSource_cmuClock1:
                frequency = IfxGtm_Cmu_getClkFrequency(gtm, IfxGtm_Cmu_Fxclk_0, TRUE);
                break;
            case IfxGtm_Dtm_ClockSource_cmuClock2:
                frequency = IfxGtm_Cmu_getClkFrequency(gtm, IfxGtm_Cmu_Fxclk_1, TRUE);
                break;
            default:
                break;
            }
        }
        else if ((dtmIndex >= IfxGtm_Dtm_4) && (dtmIndex <= IfxGtm_Dtm_5))
        {   /* Connected to ATOMs */
            switch (clockSource)
            {
            case IfxGtm_Dtm_ClockSource_systemClock:
                frequency = IfxScuCcu_getGtmFrequency();
                break;
            case IfxGtm_Dtm_ClockSource_cmuClock0:
                frequency = IfxGtm_Cmu_getClkFrequency(gtm, IfxGtm_Cmu_Clk_0, TRUE);
                break;
            case IfxGtm_Dtm_ClockSource_cmuClock1:
                frequency = IfxGtm_Cmu_getClkFrequency(gtm, IfxGtm_Cmu_Clk_1, TRUE);
                break;
            case IfxGtm_Dtm_ClockSource_cmuClock2:
                frequency = IfxGtm_Cmu_getClkFrequency(gtm, IfxGtm_Cmu_Clk_2, TRUE);
                break;
            default:
                break;
            }
        }
        else
        {}
    }

    return frequency;
}
