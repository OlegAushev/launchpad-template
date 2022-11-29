/**
 * @defgroup bsp BSP
 * 
 * @file bsp_launchxl_f28379d_def.h
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


const uint32_t j1_sciB_rxPin = 19;
const uint32_t j1_sciB_rxPinMux = GPIO_19_SCIRXDB;

const uint32_t j1_sciB_txPin = 18;
const uint32_t j1_sciB_txPinMux = GPIO_18_SCITXDB;


/// @}
} // namespace bsp


