/**
 * @file clocktasks_cpu2.cpp
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "clocktasks_cpu2.h"


///
///
///
mcu::ClockTaskStatus taskToggleLed()
{
	const uint64_t periods[4] = {100, 100, 100, 1700};
	static size_t index = 0;

	mcu::SystemClock::setTaskPeriod(0, periods[index]);
	if ((index % 2) == 0)
	{
		mcu::turnLedOn(mcu::LED_RED);
	}
	else
	{
		mcu::turnLedOff(mcu::LED_RED);
	}
	index = (index + 1) % 4;
	return mcu::ClockTaskSuccess;
}


