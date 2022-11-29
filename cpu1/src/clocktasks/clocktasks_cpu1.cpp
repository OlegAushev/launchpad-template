/**
 * @file clocktasks_cpu1.cpp
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "clocktasks_cpu1.h"


///
///
///
mcu::chrono::TaskStatus taskWatchdogTimeout()
{
	//SysLog::setError(sys::Error::CONNECTION_LOST);
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
		bsp::ledBlue.set();
	}
	else
	{
		bsp::ledBlue.reset();
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


