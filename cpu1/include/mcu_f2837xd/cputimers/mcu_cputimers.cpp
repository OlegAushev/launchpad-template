/**
 * @file
 * @ingroup mcu mcu_cpu_timers
 */


#include "mcu_cputimers.h"


namespace mcu {


volatile uint64_t SystemClock::s_time;

emb::StaticVector<SystemClock::Task, SystemClock::s_taskMaxCount> SystemClock::s_tasks;

bool SystemClock::s_watchdogEnabled;
uint64_t SystemClock::s_watchdogTimer;
uint64_t SystemClock::s_watchdogBound;
bool SystemClock::s_watchdogTimeoutDetected;
ClockTaskStatus (*SystemClock::s_watchdogTask)();

uint64_t SystemClock::s_delayedTaskStart;
uint64_t SystemClock::s_delayedTaskDelay;
void (*SystemClock::s_delayedTask)();


///
///
///
void SystemClock::init()
{
	if (initialized()) return;

	s_time = 0;

	s_watchdogEnabled = false;
	s_watchdogTimer = 0;
	s_watchdogBound = 0;
	s_watchdogTimeoutDetected = false;

	s_delayedTaskStart = 0;
	s_delayedTaskDelay = 0;

	Interrupt_register(INT_TIMER0, SystemClock::onInterrupt);

	CPUTimer_stopTimer(CPUTIMER0_BASE);		// Make sure timer is stopped
	CPUTimer_setPeriod(CPUTIMER0_BASE, 0xFFFFFFFF);	// Initialize timer period to maximum
	CPUTimer_setPreScaler(CPUTIMER0_BASE, 0);	// Initialize pre-scale counter to divide by 1 (SYSCLKOUT)
	CPUTimer_reloadTimerCounter(CPUTIMER0_BASE);	// Reload counter register with period value

	uint32_t tmp = (uint32_t)((mcu::sysclkFreq() / 1000) * s_timeStep);
	CPUTimer_setPeriod(CPUTIMER0_BASE, tmp - 1);
	CPUTimer_setEmulationMode(CPUTIMER0_BASE, CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);

	s_watchdogTask = empty_task;
	s_delayedTask = empty_delayed_task;

	CPUTimer_enableInterrupt(CPUTIMER0_BASE);
	Interrupt_enable(INT_TIMER0);
	CPUTimer_startTimer(CPUTIMER0_BASE);

	set_initialized();
}


///
///
///
void SystemClock::runTasks()
{
	for (size_t i = 0; i < s_tasks.size(); ++i)
	{
		if (now() >= (s_tasks[i].timepoint + s_tasks[i].period))
		{
			if (s_tasks[i].func(i) == ClockTaskSuccess)
			{
				s_tasks[i].timepoint = now();
			}
		}

	}

	if (s_delayedTaskDelay != 0)
	{
		if (now() >= (s_delayedTaskStart + s_delayedTaskDelay))
		{
			s_delayedTask();
			s_delayedTaskDelay = 0;
		}
	}
}


///
///
///
__interrupt void SystemClock::onInterrupt()
{
	s_time += s_timeStep;

	if (s_watchdogEnabled == true)
	{
		s_watchdogTimer += s_timeStep;
		if (s_watchdogTimer >= s_watchdogBound)
		{
			s_watchdogTimeoutDetected = true;
			if (s_watchdogTask() == ClockTaskSuccess)
			{
				resetWatchdog();
			}
		}
	}

	Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}


/*####################################################################################################################*/


uint32_t HighResolutionClock::m_period;


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

	m_period = (uint32_t)(mcu::sysclkFreq() / 1000000) * period_us - 1;
	CPUTimer_setPeriod(CPUTIMER1_BASE, m_period);
	CPUTimer_setEmulationMode(CPUTIMER1_BASE, CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);

	set_initialized();
}


} // namespace mcu


