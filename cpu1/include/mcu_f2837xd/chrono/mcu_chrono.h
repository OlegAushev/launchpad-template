/**
 * @defgroup mcu_chrono Chrono
 * @ingroup mcu
 * 
 * @file mcu_chrono.h
 * @ingroup mcu mcu_chrono
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once


#include "driverlib.h"
#include "device.h"
#include "../system/mcu_system.h"
#include "emb/emb_core.h"
#include "emb/emb_staticvector.h"


namespace mcu {


namespace chrono {
/// @addtogroup mcu_chrono
/// @{


/// Clock task statuses
SCOPED_ENUM_DECLARE_BEGIN(TaskStatus)
{
	Success = 0,
	Fail = 1
}
SCOPED_ENUM_DECLARE_END(TaskStatus)


/**
 * @brief System clock class. Based on CPU-Timer0.
 */
class SystemClock : public emb::monostate<SystemClock>
{
private:
	static volatile uint64_t _time;
	static const uint32_t _timeStep = 1;
	static const size_t _taskCountMax = 4;

/* ========================================================================== */
/* = Periodic Tasks = */
/* ========================================================================== */
private:
	struct Task
	{
		uint64_t period;
		uint64_t timepoint;
		TaskStatus (*func)(size_t);
	};
	static TaskStatus empty_task() { return TaskStatus::Success; }
	static emb::StaticVector<Task, _taskCountMax> _tasks;
public:
	/**
	 * @brief Registers periodic task.
	 * @param task - pointer to task function
	 * @param period - task period
	 * @return (none)
	 */
	static void registerTask(TaskStatus (*func)(size_t), uint64_t period)
	{
		Task task = {period, now(), func};
		_tasks.push_back(task);
	}

	/**
	 * @brief Set task period.
	 * @param index - task index
	 * @param period - task period in milliseconds
	 * @return (none)
	 */
	static void setTaskPeriod(size_t index, uint64_t period)
	{
		if (index < _tasks.size())
		{
			_tasks[index].period = period;
		}
	}

/* ========================================================================== */
/* = Watchdog = */
/* ========================================================================== */
private:
	static bool _watchdogEnabled;
	static uint64_t _watchdogTimer;
	static uint64_t _watchdogBound;
	static bool _watchdogTimeoutDetected;
	static TaskStatus (*_watchdogTask)();
public:
	/**
	 * @brief Enable watchdog.
	 * @param (none)
	 * @return (none)
	 */
	static void enableWatchdog()
	{
		_watchdogEnabled = true;
	}

	/**
	 * @brief Disables watchdog.
	 * @param (none)
	 * @return (none)
	 */
	static void disableWatchdog()
	{
		_watchdogEnabled = false;
	}

	/**
	 * @brief Sets watchdog bound.
	 * @param watchdogPeriod_ms - bound in milliseconds
	 * @return (none)
	 */
	static void setWatchdogBound(uint64_t watchdogBound_ms)
	{
		_watchdogBound = watchdogBound_ms;
	}

	/**
	 * @brief Resets watchdog timer.
	 * @param (none)
	 * @return (none)
	 */
	static void resetWatchdogTimer()
	{
		_watchdogTimer = 0;
	}

	/**
	 * @brief Checks if watchdog timeout is detected.
	 * @param (none)
	 * @return \c true if timeout is detected, \c false otherwise.
	 */
	static bool watchdogTimeoutDetected()
	{
		return _watchdogTimeoutDetected;
	}

	/**
	 * @brief Resets watchdog.
	 * @param (none)
	 * @return (none)
	 */
	static void resetWatchdog()
	{
		_watchdogTimeoutDetected = false;
		resetWatchdogTimer();
	}

	/**
	 * @brief Registers watchdog timeout task.
	 * @param task - pointer to task function
	 * @return (none)
	 */
	static void registerWatchdogTask(TaskStatus (*task)(), uint64_t watchdogBound_ms)
	{
		_watchdogTask = task;
		_watchdogBound = watchdogBound_ms;
	}

/* ========================================================================== */
/* = Delayed Task = */
/* ========================================================================== */
private:
	static uint64_t _delayedTaskStart;
	static uint64_t _delayedTaskDelay;
	static void (*_delayedTask)();
	static void empty_delayed_task() {}
public:
	/**
	 * @brief Registers delayed task.
	 * @param task - pointer to delayed task function
	 * @return (none)
	 */
	static void registerDelayedTask(void (*task)(), uint64_t delay)
	{
		_delayedTask = task;
		_delayedTaskDelay = delay;
		_delayedTaskStart = now();
	}

private:
	SystemClock();						// no constructor
	SystemClock(const SystemClock& other);			// no copy constructor
	SystemClock& operator=(const SystemClock& other);	// no copy assignment operator
public:
	/**
	 * @brief Initializes clock.
	 * @param (none)
	 * @return (none)
	 */
	static void init();

	/**
	 * @brief Returns a time point representing the current point in time.
	 * @param (none)
	 * @return A time point representing the current time in milliseconds.
	 */
	static uint64_t now()
	{
		return _time;
	}

	/**
	 * @brief Returns clock step.
	 * @param (none)
	 * @return Clock step in milliseconds.
	 */
	static uint32_t step()
	{
		return _timeStep;
	}

	/**
	 * @brief Resets clock.
	 * @param (none)
	 * @return (none)
	 */
	static void reset()
	{
		_time = 0;
		for (size_t i = 0; i < _tasks.size(); ++i)
		{
			_tasks[i].timepoint = now();
		}
	}

	/**
	 * @brief Checks and runs periodic and delayed tasks.
	 * @param (none)
	 * @return (none)
	 */
	static void runTasks();

protected:
	/**
	 * @brief Main system clock ISR.
	 * @param (none)
	 * @return (none)
	 */
	static __interrupt void onInterrupt();
};


/*####################################################################################################################*/
/**
 * @brief High resolution clock class. Based on CPU-Timer1.
 */
class HighResolutionClock : public emb::monostate<HighResolutionClock>
{
private:
	static uint32_t _period;
	static const uint32_t deviceSysclkPeriod_ns = 1000000000 / DEVICE_SYSCLK_FREQ;
public:
	/**
	 * @brief Initializes systick timer.
	 * @param (none)
	 * @return (none)
	 */
	static void init(uint32_t period_us);

	/**
	 * @brief Returns systick timer counter value.
	 * @param (none)
	 * @return Systick timer counter value.
	 */
	static uint32_t counter()
	{
		return CPUTimer_getTimerCount(CPUTIMER1_BASE);
	}

	/**
	 * @brief Returns a time point representing the current point in time.
	 * @param (none)
	 * @return A time point representing the current time in ns.
	 */
	static uint64_t now()
	{
		return static_cast<uint64_t>(_period - counter()) * deviceSysclkPeriod_ns;
	}

	/**
	 * @brief Starts systick timer.
	 * @param (none)
	 * @return (none)
	 */
	static void start()
	{
		CPUTimer_startTimer(CPUTIMER1_BASE);
	}

	/**
	 * @brief Stops systick timer.
	 * @param (none)
	 * @return (none)
	 */
	static void stop()
	{
		CPUTimer_stopTimer(CPUTIMER1_BASE);
	}

	/**
	 * @brief Registers systick timer interrupt handler.
	 * @param handler - pointer to handler
	 * @return (none)
	 */
	static void registerInterruptHandler(void (*handler)(void))
	{
		Interrupt_register(INT_TIMER1, handler);
		CPUTimer_enableInterrupt(CPUTIMER1_BASE);
		Interrupt_enable(INT_TIMER1);
	}

private:
	static __interrupt void onInterrupt();
};


/*####################################################################################################################*/
/**
 * @brief Timeout watchdog class.
 */
class Timeout
{
private:
	const uint64_t _timeout;
	volatile uint64_t _start;
public:
	/**
	 * @brief
	 * @param timeout - timeout in milliseconds
	 * @return (none)
	 */
	Timeout(uint64_t timeout = 0)
		: _timeout(timeout)
		, _start(SystemClock::now())
	{}

	/**
	 * @brief Check if timeout is expired.
	 * @param (none)
	 * @return \c true if timeout is expired, \c false otherwise.
	 */
	bool expired() volatile
	{
		if (_timeout == 0)
		{
			return false;
		}
		if ((SystemClock::now() - _start) > _timeout)
		{
			return true;
		}
		return false;
	}

	/**
	 * @brief Resets timeout watchdog.
	 * @param (none)
	 * @return (none)
	 */
	void reset() volatile
	{
		_start = SystemClock::now();
	}
};


/// @}
} // namespace chrono


} // namespace mcu


