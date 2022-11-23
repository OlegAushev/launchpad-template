/**
 * @file
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


