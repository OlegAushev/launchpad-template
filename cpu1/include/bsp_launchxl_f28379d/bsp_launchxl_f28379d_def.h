///
#pragma once


#include "mcu_f2837xd/gpio/mcu_gpio.h"


namespace bsp {


const uint32_t j1_sciB_rxPin = 19;
const uint32_t j1_sciB_rxPinMux = GPIO_19_SCIRXDB;

const uint32_t j1_sciB_txPin = 18;
const uint32_t j1_sciB_txPinMux = GPIO_18_SCITXDB;


} // namespace bsp


