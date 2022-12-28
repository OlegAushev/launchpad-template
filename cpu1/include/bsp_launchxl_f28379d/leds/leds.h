/**
 * @file leds.h
 * @ingroup bsp
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once


#include "mcu_f2837xd/gpio/mcu_gpio.h"


namespace bsp {
/// @addtogroup bsp
/// @{


extern mcu::gpio::Output ledBlue;
extern const mcu::gpio::Config ledBlueConfig;

extern mcu::gpio::Output ledRed;
extern const mcu::gpio::Config ledRedConfig;


/**
 * @brief 
 * 
 * @param core 
 */
void initLedBlue(mcu::gpio::MasterCore core = mcu::gpio::MasterCore::Cpu1);


/**
 * @brief 
 * 
 * @param core 
 */
void initLedRed(mcu::gpio::MasterCore core = mcu::gpio::MasterCore::Cpu1);


/// @}
} // namespace bsp


