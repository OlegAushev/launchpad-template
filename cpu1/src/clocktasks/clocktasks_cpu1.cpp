/**
 * @file
 */


#include "clocktasks_cpu1.h"


///
///
///
mcu::chrono::TaskStatus taskWatchdogTimeout()
{
	//Syslog::setError(sys::Error::CONNECTION_LOST);
	return mcu::chrono::TaskStatus::Success;
}


///
///
///
mcu::chrono::TaskStatus taskToggleLed(size_t taskIndex)
{
	const uint64_t periods[4] = {100, 100, 100, 1700};
	static size_t index = 0;

	mcu::chrono::SystemClock::setTaskPeriod(taskIndex, periods[index]);
	if ((index % 2) == 0)
	{
		mcu::turnLedOn(mcu::LED_BLUE);
	}
	else
	{
		mcu::turnLedOff(mcu::LED_BLUE);
	}
	index = (index + 1) % 4;
	return mcu::chrono::TaskStatus::Success;
}


///
///
///
mcu::chrono::TaskStatus taskStartTempSensorAdc()
{
	return mcu::chrono::TaskStatus::Success;
}


