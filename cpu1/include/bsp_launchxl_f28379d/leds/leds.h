///
#pragma once


#include "mcu_f2837xd/gpio/mcu_gpio.h"


namespace bsp {


extern mcu::gpio::Output ledBlue;
extern const mcu::gpio::Config ledBlueConfig;

extern mcu::gpio::Output ledRed;
extern const mcu::gpio::Config ledRedConfig;

void initLedBlue(mcu::gpio::MasterCore core = mcu::gpio::MasterCore::Cpu1);
void initLedRed(mcu::gpio::MasterCore core = mcu::gpio::MasterCore::Cpu1);


} // namespace bsp


