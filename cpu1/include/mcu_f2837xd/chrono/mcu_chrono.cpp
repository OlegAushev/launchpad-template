/**
 * @file mcu_chrono.cpp
 * @ingroup mcu mcu_chrono
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "mcu_chrono.h"


namespace mcu {


namespace chrono {


volatile uint64_t SystemClock::_time;

emb::StaticVector<SystemClock::Task, SystemClock::_taskCountMax> SystemClock::_tasks;

bool SystemClock::_watchdogEnabled;
uint64_t SystemClock::_watchdogTimer;
uint64_t SystemClock::_watchdogBound;
bool SystemClock::_watchdogTimeoutDetected;
TaskStatus (*SystemClock::_watchdogTask)();

uint64_t SystemClock::_delayedTaskStart;
uint64_t SystemClock::_delayedTaskDelay;
void (*SystemClock::_delayedTask)();


///
///
///
void SystemClock::init()
{
	if (initialized()) return;

	_time = 0;

	_watchdogEnabled = false;
	_watchdogTimer = 0;
	_watchdogBound = 0;
	_watchdogTimeoutDetected = false;

	_delayedTaskStart = 0;
	_delayedTaskDelay = 0;

	Interrupt_register(INT_TIMER0, SystemClock::onInterrupt);

	CPUTimer_stopTimer(CPUTIMER0_BASE);		// Make sure timer is stopped
	CPUTimer_setPeriod(CPUTIMER0_BASE, 0xFFFFFFFF);	// Initialize timer period to maximum
	CPUTimer_setPreScaler(CPUTIMER0_BASE, 0);	// Initialize pre-scale counter to divide by 1 (SYSCLKOUT)
	CPUTimer_reloadTimerCounter(CPUTIMER0_BASE);	// Reload counter register with period value

	uint32_t tmp = (uint32_t)((mcu::sysclkFreq() / 1000) * _timeStep);
	CPUTimer_setPeriod(CPUTIMER0_BASE, tmp - 1);
	CPUTimer_setEmulationMode(CPUTIMER0_BASE, CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);

	_watchdogTask = empty_task;
	_delayedTask = empty_delayed_task;

	CPUTimer_enableInterrupt(CPUTIMER0_BASE);
	Interrupt_enable(INT_TIMER0);
	CPUTimer_startTimer(CPUTIMER0_BASE);

	_set_initialized();
}


///
///
///
void SystemClock::runTasks()
{
	for (size_t i = 0; i < _tasks.size(); ++i)
	{
		if (now() >= (_tasks[i].timepoint + _tasks[i].period))
		{
			if (_tasks[i].func(i) == TaskStatus::Success)
			{
				_tasks[i].timepoint = now();
			}
		}

	}

	if (_delayedTaskDelay != 0)
	{
		if (now() >= (_delayedTaskStart + _delayedTaskDelay))
		{
			_delayedTask();
			_delayedTaskDelay = 0;
		}
	}
}


///
///
///
__interrupt void SystemClock::onInterrupt()
{
	_time += _timeStep;

	if (_watchdogEnabled == true)
	{
		_watchdogTimer += _timeStep;
		if (_watchdogTimer >= _watchdogBound)
		{
			_watchdogTimeoutDetected = true;
			if (_watchdogTask() == TaskStatus::Success)
			{
				resetWatchdog();
			}
		}
	}

	Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}


/*####################################################################################################################*/


uint32_t HighResolutionClock::_period;


///
///
///
void HighResolutionClock::init(uint32_t period_us)
{
	if (initialized()) return;

	CPUTimer_stopTimer(CPUTIMER1_BASE);             	// Make sure timer is stopped
	CPUTimer_setPeriod(CPUTIMER1_BASE, 0xFFFFFFFF); 	// Initialize timer period to maximum
	CPUTimer_setPreScaler(CPUTIMER1_BASE, 0);       	// Initialize pre-scale counter to divide by 1 (SYSCLKOUT)
	CPUTimer_reloadTimerCounter(CPUTIMER1_BASE);    	// Reload counter register with period value

	_period = (uint32_t)(mcu::sysclkFreq() / 1000000) * period_us - 1;
	CPUTimer_setPeriod(CPUTIMER1_BASE, _period);
	CPUTimer_setEmulationMode(CPUTIMER1_BASE, CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);

	_set_initialized();
}


} // namespace chrono


} // namespace mcu


