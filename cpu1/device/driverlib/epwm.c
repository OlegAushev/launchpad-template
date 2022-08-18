//###########################################################################
//
// FILE:   epwm.c
//
// TITLE:  C28x EPWM driver.
//
//###########################################################################
// $TI Release: F2837xD Support Library v3.11.00.00 $
// $Release Date: Sun Oct  4 15:55:24 IST 2020 $
// $Copyright:
// Copyright (C) 2013-2020 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//###########################################################################

#include "epwm.h"

//*****************************************************************************
//
// EPWM_setEmulationMode
//
//*****************************************************************************
void EPWM_setEmulationMode(uint32_t base, EPWM_EmulationMode emulationMode)
{
    //
    // Check the arguments.
    //
    ASSERT(EPWM_isBaseValid(base));

    //
    // Write to FREE_SOFT bits
    //
    HWREGH(base + EPWM_O_TBCTL) =
           ((HWREGH(base + EPWM_O_TBCTL) & (~EPWM_TBCTL_FREE_SOFT_M)) |
            ((uint16_t)emulationMode << EPWM_TBCTL_FREE_SOFT_S));
}

//*****************************************************************************
//
// EPWM_configureSignal
//
//*****************************************************************************
void EPWM_configureSignal(uint32_t base, const EPWM_SignalParams *signalParams)
{
    float32_t tbClkInHz = 0.0F;
    uint16_t tbPrdVal = 0U, cmpAVal = 0U, cmpBVal = 0U;

    //
    // Check the arguments.
    //
    ASSERT(EPWM_isBaseValid(base));

    //
    // Configure EPWM clock Divider
    //
    SysCtl_setEPWMClockDivider(signalParams->epwmClkDiv);

    //
    // Configure Time Base counter Clock
    //
    EPWM_setClockPrescaler(base, signalParams->tbClkDiv,
                           signalParams->tbHSClkDiv);

    //
    // Configure Time Base Counter Mode
    //
    EPWM_setTimeBaseCounterMode(base, signalParams->tbCtrMode);

    //
    // Calculate TBCLK, TBPRD and CMPx values to be configured for
    // achieving desired signal
    //
    tbClkInHz = ((float32_t)signalParams->sysClkInHz /
                (1U << ((uint16_t)signalParams->epwmClkDiv +
                 (uint16_t)signalParams->tbClkDiv)));

    if(signalParams->tbHSClkDiv <= EPWM_HSCLOCK_DIVIDER_4)
    {
        tbClkInHz /= (1U << (uint16_t)signalParams->tbHSClkDiv);
    }
    else
    {
        tbClkInHz /= (2U * (uint16_t)signalParams->tbHSClkDiv);
    }

    if(signalParams->tbCtrMode == EPWM_COUNTER_MODE_UP)
    {
        tbPrdVal = (uint16_t)((tbClkInHz / signalParams->freqInHz) - 1.0f);
        cmpAVal = (uint16_t)((float32_t)signalParams->dutyValA *
                             (tbPrdVal + 1U));
        cmpBVal = (uint16_t)((float32_t)signalParams->dutyValB *
                             (tbPrdVal + 1U));
    }
    else if(signalParams->tbCtrMode == EPWM_COUNTER_MODE_DOWN)
    {
        tbPrdVal = (uint16_t)((tbClkInHz / signalParams->freqInHz) - 1.0f);
        cmpAVal = (uint16_t)((tbPrdVal + 1U) -
                       ((float32_t)signalParams->dutyValA * (tbPrdVal + 1U)));
        cmpBVal = (uint16_t)((tbPrdVal + 1U) -
                       ((float32_t)signalParams->dutyValB * (tbPrdVal + 1U)));
    }
    else if((signalParams->tbCtrMode == EPWM_COUNTER_MODE_UP_DOWN))
    {
        tbPrdVal = (uint16_t)(tbClkInHz / (2.0f * signalParams->freqInHz));
        cmpAVal = (uint16_t)(((float32_t)tbPrdVal -
                             ((float32_t)(signalParams->dutyValA *
                              tbPrdVal))) + 0.5f);
        cmpBVal = (uint16_t)(((float32_t)tbPrdVal -
                             ((float32_t)(signalParams->dutyValB *
                              tbPrdVal))) + 0.5f);
    }

    //
    // Configure TBPRD value
    //
    EPWM_setTimeBasePeriod(base, tbPrdVal);

    //
    // Default Configurations.
    //
    EPWM_disablePhaseShiftLoad(base);
    EPWM_setPhaseShift(base, 0U);
    EPWM_setTimeBaseCounter(base, 0U);

    //
    // Setup shadow register load on ZERO
    //
    EPWM_setCounterCompareShadowLoadMode(base,
                                         EPWM_COUNTER_COMPARE_A,
                                         EPWM_COMP_LOAD_ON_CNTR_ZERO);
    EPWM_setCounterCompareShadowLoadMode(base,
                                         EPWM_COUNTER_COMPARE_B,
                                         EPWM_COMP_LOAD_ON_CNTR_ZERO);
    //
    // Set Compare values
    //
    EPWM_setCounterCompareValue(base, EPWM_COUNTER_COMPARE_A,
                                cmpAVal);
    EPWM_setCounterCompareValue(base, EPWM_COUNTER_COMPARE_B,
                                cmpBVal);

    //
    // Set actions for ePWMxA & ePWMxB
    //
    if(signalParams->tbCtrMode == EPWM_COUNTER_MODE_UP)
    {
        //
        // Set PWMxA on Zero
        //
        EPWM_setActionQualifierAction(base,
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_HIGH,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);

        //
        // Clear PWMxA on event A, up count
        //
        EPWM_setActionQualifierAction(base,
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

        if(signalParams->invertSignalB == true)
        {
            //
            // Clear PWMxB on Zero
            //
            EPWM_setActionQualifierAction(base,
                                          EPWM_AQ_OUTPUT_B,
                                          EPWM_AQ_OUTPUT_LOW,
                                          EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
            //
            // Set PWMxB on event B, up count
            //
            EPWM_setActionQualifierAction(base,
                                          EPWM_AQ_OUTPUT_B,
                                          EPWM_AQ_OUTPUT_HIGH,
                                          EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);
        }
        else
        {
            //
            // Set PWMxB on Zero
            //
            EPWM_setActionQualifierAction(base,
                                          EPWM_AQ_OUTPUT_B,
                                          EPWM_AQ_OUTPUT_HIGH,
                                          EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
            //
            // Clear PWMxB on event B, up count
            //
            EPWM_setActionQualifierAction(base,
                                          EPWM_AQ_OUTPUT_B,
                                          EPWM_AQ_OUTPUT_LOW,
                                          EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);

        }
    }
    else if((signalParams->tbCtrMode == EPWM_COUNTER_MODE_DOWN))
    {
        //
        // Set PWMxA on Zero
        //
        EPWM_setActionQualifierAction(base,
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_HIGH,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);

        //
        // Clear PWMxA on event A, down count
        //
        EPWM_setActionQualifierAction(base,
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

        if(signalParams->invertSignalB == true)
        {
            //
            // Clear PWMxB on Zero
            //
            EPWM_setActionQualifierAction(base,
                                          EPWM_AQ_OUTPUT_B,
                                          EPWM_AQ_OUTPUT_LOW,
                                          EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
            //
            // Set PWMxB on event B, down count
            //
            EPWM_setActionQualifierAction(base,
                                          EPWM_AQ_OUTPUT_B,
                                          EPWM_AQ_OUTPUT_HIGH,
                                          EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);
        }
        else
        {
            //
            // Set PWMxB on Zero
            //
            EPWM_setActionQualifierAction(base,
                                          EPWM_AQ_OUTPUT_B,
                                          EPWM_AQ_OUTPUT_HIGH,
                                          EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
            //
            // Clear PWMxB on event B, down count
            //
            EPWM_setActionQualifierAction(base,
                                          EPWM_AQ_OUTPUT_B,
                                          EPWM_AQ_OUTPUT_LOW,
                                          EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);
        }
    }
    else if(signalParams->tbCtrMode == EPWM_COUNTER_MODE_UP_DOWN)
    {
        //
        // Clear PWMxA on Zero
        //
        EPWM_setActionQualifierAction(base,
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);

        //
        // Set PWMxA on event A, up count
        //
        EPWM_setActionQualifierAction(base,
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_HIGH,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

        //
        // Clear PWMxA on event A, down count
        //
        EPWM_setActionQualifierAction(base,
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

        if(signalParams->invertSignalB == true)
        {
            //
            // Set PWMxB on Zero
            //
            EPWM_setActionQualifierAction(base,
                                          EPWM_AQ_OUTPUT_B,
                                          EPWM_AQ_OUTPUT_HIGH,
                                          EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);

            //
            // Clear PWMxB on event B, up count
            //
            EPWM_setActionQualifierAction(base,
                                          EPWM_AQ_OUTPUT_B,
                                          EPWM_AQ_OUTPUT_LOW,
                                          EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);
            //
            // Set PWMxB on event B, down count
            //
            EPWM_setActionQualifierAction(base,
                                          EPWM_AQ_OUTPUT_B,
                                          EPWM_AQ_OUTPUT_HIGH,
                                          EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);
        }
        else
        {
            //
            // Clear PWMxB on Zero
            //
            EPWM_setActionQualifierAction(base,
                                          EPWM_AQ_OUTPUT_B,
                                          EPWM_AQ_OUTPUT_LOW,
                                          EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);

            //
            // Set PWMxB on event B, up count
            //
            EPWM_setActionQualifierAction(base,
                                          EPWM_AQ_OUTPUT_B,
                                          EPWM_AQ_OUTPUT_HIGH,
                                          EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);
            //
            // Clear PWMxB on event B, down count
            //
            EPWM_setActionQualifierAction(base,
                                          EPWM_AQ_OUTPUT_B,
                                          EPWM_AQ_OUTPUT_LOW,
                                          EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);
        }
    }
}

