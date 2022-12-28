/**
 * @file leds.cpp
 * @ingroup bsp
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "leds.h"


namespace bsp {


mcu::gpio::Output ledBlue;
extern const mcu::gpio::Config ledBlueConfig(
	31,
	GPIO_31_GPIO31,
	mcu::gpio::Direction::Output,
	emb::gpio::ActiveState::Low,
	mcu::gpio::Type::Std,
	mcu::gpio::QualMode::Sync,
	1
);


mcu::gpio::Output ledRed;
extern const mcu::gpio::Config ledRedConfig(
	34,
	GPIO_34_GPIO34,
	mcu::gpio::Direction::Output,
	emb::gpio::ActiveState::Low,
	mcu::gpio::Type::Std,
	mcu::gpio::QualMode::Sync,
	1
);


void initLedBlue(mcu::gpio::MasterCore core)
{
	ledBlue.init(ledBlueConfig);
	ledBlue.setMasterCore(core);
}


void initLedRed(mcu::gpio::MasterCore core)
{
	ledRed.init(ledRedConfig);
	ledRed.setMasterCore(core);
}


} // namespace bsp


