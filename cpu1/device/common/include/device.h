//#############################################################################
//
// FILE:   device.h
//
// TITLE:  Device setup for examples.
//
//#############################################################################
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
//#############################################################################

//
// Included Files
//
#include "driverlib.h"

#if (!defined(CPU1) && !defined(CPU2))
#error "You must define CPU1 or CPU2 in your project properties.  Otherwise, \
the offsets in your header files will be inaccurate."
#endif

#if (defined(CPU1) && defined(CPU2))
#error "You have defined both CPU1 and CPU2 in your project properties.  Only \
a single CPU should be defined."
#endif

//*****************************************************************************
//
// Defines for pin numbers and other GPIO configuration
//
//*****************************************************************************
//
// LEDs
//
#ifdef _LAUNCHXL_F28379D
#define DEVICE_GPIO_PIN_LED1        31U  // GPIO number for LD10
#define DEVICE_GPIO_PIN_LED2        34U  // GPIO number for LD9
#define DEVICE_GPIO_CFG_LED1        GPIO_31_GPIO31  // "pinConfig" for LD10
#define DEVICE_GPIO_CFG_LED2        GPIO_34_GPIO34  // "pinConfig" for LD9
#else
#define DEVICE_GPIO_PIN_LED1        31U  // GPIO number for LD2
#define DEVICE_GPIO_PIN_LED2        34U  // GPIO number for LD3
#define DEVICE_GPIO_CFG_LED1        GPIO_31_GPIO31  // "pinConfig" for LD2
#define DEVICE_GPIO_CFG_LED2        GPIO_34_GPIO34  // "pinConfig" for LD3
#endif


//
// SCI for USB-to-UART adapter on FTDI chip
//
#ifdef _LAUNCHXL_F28379D
#define DEVICE_GPIO_PIN_SCIRXDA     43U             // GPIO number for SCI RX
#define DEVICE_GPIO_PIN_SCITXDA     42U             // GPIO number for SCI TX
#define DEVICE_GPIO_CFG_SCIRXDA     GPIO_43_SCIRXDA // "pinConfig" for SCI RX
#define DEVICE_GPIO_CFG_SCITXDA     GPIO_42_SCITXDA // "pinConfig" for SCI TX
#else
#define DEVICE_GPIO_PIN_SCIRXDA     28U             // GPIO number for SCI RX
#define DEVICE_GPIO_PIN_SCITXDA     29U             // GPIO number for SCI TX
#define DEVICE_GPIO_CFG_SCIRXDA     GPIO_28_SCIRXDA // "pinConfig" for SCI RX
#define DEVICE_GPIO_CFG_SCITXDA     GPIO_29_SCITXDA // "pinConfig" for SCI TX
#endif

//
// CANA
//
#define DEVICE_GPIO_PIN_CANTXA      31U  // GPIO number for CANTXA
#define DEVICE_GPIO_PIN_CANRXA      30U  // GPIO number for CANRXA

//
// CAN External Loopback
//
#define DEVICE_GPIO_CFG_CANRXA      GPIO_30_CANRXA  // "pinConfig" for CANA RX
#define DEVICE_GPIO_CFG_CANTXA      GPIO_31_CANTXA  // "pinConfig" for CANA TX
#define DEVICE_GPIO_CFG_CANRXB      GPIO_10_CANRXB  // "pinConfig" for CANB RX
#define DEVICE_GPIO_CFG_CANTXB      GPIO_8_CANTXB   // "pinConfig" for CANB TX

//*****************************************************************************
//
// Defines related to clock configuration
//
//*****************************************************************************
//
// Launchpad Configuration
//
#ifdef _LAUNCHXL_F28379D

//
// 10MHz XTAL on LaunchPad. For use with SysCtl_getClock().
//
#define DEVICE_OSCSRC_FREQ          10000000U

//
// Define to pass to SysCtl_setClock(). Will configure the clock as follows:
// PLLSYSCLK = 10MHz (XTAL_OSC) * 40 (IMULT) * 1 (FMULT) / 2 (PLLCLK_BY_2)
//
#define DEVICE_SETCLOCK_CFG         (SYSCTL_OSCSRC_XTAL | SYSCTL_IMULT(40) |  \
                                     SYSCTL_FMULT_NONE | SYSCTL_SYSDIV(2) |   \
                                     SYSCTL_PLL_ENABLE)

//
// 200MHz SYSCLK frequency based on the above DEVICE_SETCLOCK_CFG. Update the
// code below if a different clock configuration is used!
//
#define DEVICE_SYSCLK_FREQ          ((DEVICE_OSCSRC_FREQ * 40 * 1) / 2)

//
// ControlCARD Configuration
//
#else

//
// 20MHz XTAL on controlCARD. For use with SysCtl_getClock().
//
#define DEVICE_OSCSRC_FREQ          20000000U

//
// Define to pass to SysCtl_setClock(). Will configure the clock as follows:
// PLLSYSCLK = 20MHz (XTAL_OSC) * 20 (IMULT) * 1 (FMULT) / 2 (PLLCLK_BY_2)
//
#define DEVICE_SETCLOCK_CFG         (SYSCTL_OSCSRC_XTAL | SYSCTL_IMULT(20) |  \
                                     SYSCTL_FMULT_NONE | SYSCTL_SYSDIV(2) |   \
                                     SYSCTL_PLL_ENABLE)

//
// 200MHz SYSCLK frequency based on the above DEVICE_SETCLOCK_CFG. Update the
// code below if a different clock configuration is used!
//
#define DEVICE_SYSCLK_FREQ          ((DEVICE_OSCSRC_FREQ * 20 * 1) / 2)

#endif

//
// 50MHz LSPCLK frequency based on the above DEVICE_SYSCLK_FREQ and a default
// low speed peripheral clock divider of 4. Update the code below if a
// different LSPCLK divider is used!
//
#define DEVICE_LSPCLK_FREQ          (DEVICE_SYSCLK_FREQ / 4)

//*****************************************************************************
//
// Macro to call SysCtl_delay() to achieve a delay in microseconds. The macro
// will convert the desired delay in microseconds to the count value expected
// by the function. \b x is the number of microseconds to delay.
//
//*****************************************************************************
#define DEVICE_DELAY_US(x) SysCtl_delay(((((long double)(x)) / (1000000.0L /  \
                              (long double)DEVICE_SYSCLK_FREQ)) - 9.0L) / 5.0L)

//
// The macros that can be used as parameter to the function Device_bootCPU2
//
#define C1C2_BROM_BOOTMODE_BOOT_FROM_PARALLEL                        0x00000000U
#define C1C2_BROM_BOOTMODE_BOOT_FROM_SCI                             0x00000001U
#define C1C2_BROM_BOOTMODE_BOOT_FROM_SPI                             0x00000004U
#define C1C2_BROM_BOOTMODE_BOOT_FROM_I2C                             0x00000005U
#define C1C2_BROM_BOOTMODE_BOOT_FROM_CAN                             0x00000007U
#define C1C2_BROM_BOOTMODE_BOOT_FROM_RAM                             0x0000000AU
#define C1C2_BROM_BOOTMODE_BOOT_FROM_FLASH                           0x0000000BU

//
// Other macros that are needed for the Device_bootCPU2 function
//
#define BROM_IPC_EXECUTE_BOOTMODE_CMD                                0x00000013U
#define C1C2_BROM_BOOTMODE_BOOT_COMMAND_MAX_SUPPORT_VALUE            0x0000000CU
#define C2_BOOTROM_BOOTSTS_C2TOC1_IGNORE                             0x00000000U
#define C2_BOOTROM_BOOTSTS_SYSTEM_START_BOOT                         0x00000001U
#define C2_BOOTROM_BOOTSTS_SYSTEM_READY                              0x00000002U
#define C2_BOOTROM_BOOTSTS_C2TOC1_BOOT_CMD_ACK                       0x00000003U
#define C2_BOOTROM_BOOTSTS_C2TOC1_BOOT_CMD_NAK_STATUS_NOT_SUPPORTED  0x00000004U
#define C2_BOOTROM_BOOTSTS_C2TOC1_BOOT_CMD_NAK_STATUS_BUSY_WITH_BOOT 0x00000005U

//
// Macros used as return value by the Device_bootCPU2 function
//
#define STATUS_FAIL                 0x0001
#define STATUS_PASS                 0x0000

//*****************************************************************************
//
// Defines, Globals, and Header Includes related to Flash Support
//
//*****************************************************************************
#ifdef _FLASH
#include <stddef.h>

extern uint16_t RamfuncsLoadStart;
extern uint16_t RamfuncsLoadEnd;
extern uint16_t RamfuncsLoadSize;
extern uint16_t RamfuncsRunStart;
extern uint16_t RamfuncsRunEnd;
extern uint16_t RamfuncsRunSize;

#define DEVICE_FLASH_WAITSTATES 3

#endif

extern uint32_t Example_PassCount;
extern uint32_t Example_Fail;

//*****************************************************************************
//
// Function Prototypes
//
//*****************************************************************************
extern void Device_init(void);
extern void Device_enableAllPeripherals(void);
extern void Device_initGPIO(void);
extern void Device_enableUnbondedGPIOPullupsFor176Pin(void);
extern void Device_enableUnbondedGPIOPullupsFor100Pin(void);
extern void Device_enableUnbondedGPIOPullups(void);
#ifdef CPU1
extern void Device_configureTMXAnalogTrim(void);
extern uint16_t Device_bootCPU2(uint32_t ulBootMode);
#endif
extern void __error__(const char *filename, uint32_t line);
extern void Example_setResultPass(void);
extern void Example_setResultFail(void);
extern void Example_done(void);

//
// End of file
//
