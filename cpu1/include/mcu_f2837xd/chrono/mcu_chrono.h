/**
 * @defgroup mcu_cpu_timers CPU Timers
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_cpu_timers
 */


#pragma once


#include "driverlib.h"
#include "device.h"
#include "../system/mcu_system.h"
#include "emb/emb_core.h"
#include "emb/emb_staticvector.h"


namespace mcu {


namespace chrono {
/// @addtogroup mcu_cpu_timers
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
	static volatile uint64_t s_time;
	static const uint32_t s_timeStep = 1;
	static const size_t s_taskMaxCount = 4;

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
	static emb::StaticVector<Task, s_taskMaxCount> s_tasks;
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
		s_tasks.push_back(task);
	}

	/**
	 * @brief Set task period.
	 * @param index - task index
	 * @param period - task period in milliseconds
	 * @return (none)
	 */
	static void setTaskPeriod(size_t index, uint64_t period)
	{
		if (index < s_tasks.size())
		{
			s_tasks[index].period = period;
		}
	}

/* ========================================================================== */
/* = Watchdog = */
/* ========================================================================== */
private:
	static bool s_watchdogEnabled;
	static uint64_t s_watchdogTimer;
	static uint64_t s_watchdogBound;
	static bool s_watchdogTimeoutDetected;
	static TaskStatus (*s_watchdogTask)();
public:
	/**
	 * @brief Enable watchdog.
	 * @param (none)
	 * @return (none)
	 */
	static void enableWatchdog()
	{
		s_watchdogEnabled = true;
	}

	/**
	 * @brief Disables watchdog.
	 * @param (none)
	 * @return (none)
	 */
	static void disableWatchdog()
	{
		s_watchdogEnabled = false;
	}

	/**
	 * @brief Sets watchdog bound.
	 * @param watchdogPeriod_ms - bound in milliseconds
	 * @return (none)
	 */
	static void setWatchdogBound(uint64_t watchdogBound_ms)
	{
		s_watchdogBound = watchdogBound_ms;
	}

	/**
	 * @brief Resets watchdog timer.
	 * @param (none)
	 * @return (none)
	 */
	static void resetWatchdogTimer()
	{
		s_watchdogTimer = 0;
	}

	/**
	 * @brief Checks if watchdog timeout is detected.
	 * @param (none)
	 * @return \c true if timeout is detected, \c false otherwise.
	 */
	static bool watchdogTimeoutDetected()
	{
		return s_watchdogTimeoutDetected;
	}

	/**
	 * @brief Resets watchdog.
	 * @param (none)
	 * @return (none)
	 */
	static void resetWatchdog()
	{
		s_watchdogTimeoutDetected = false;
		resetWatchdogTimer();
	}

	/**
	 * @brief Registers watchdog timeout task.
	 * @param task - pointer to task function
	 * @return (none)
	 */
	static void registerWatchdogTask(TaskStatus (*task)(), uint64_t watchdogBound_ms)
	{
		s_watchdogTask = task;
		s_watchdogBound = watchdogBound_ms;
	}

/* ========================================================================== */
/* = Delayed Task = */
/* ========================================================================== */
private:
	static uint64_t s_delayedTaskStart;
	static uint64_t s_delayedTaskDelay;
	static void (*s_delayedTask)();
	static void empty_delayed_task() {}
public:
	/**
	 * @brief Registers delayed task.
	 * @param task - pointer to delayed task function
	 * @return (none)
	 */
	static void registerDelayedTask(void (*task)(), uint64_t delay)
	{
		s_delayedTask = task;
		s_delayedTaskDelay = delay;
		s_delayedTaskStart = now();
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
		return s_time;
	}

	/**
	 * @brief Returns clock step.
	 * @param (none)
	 * @return Clock step in milliseconds.
	 */
	static uint32_t step()
	{
		return s_timeStep;
	}

	/**
	 * @brief Resets clock.
	 * @param (none)
	 * @return (none)
	 */
	static void reset()
	{
		s_time = 0;
		for (size_t i = 0; i < s_tasks.size(); ++i)
		{
			s_tasks[i].timepoint = now();
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
	static uint32_t m_period;
	static const uint32_t DEVICE_SYSCLK_PERIOD_NS = 1000000000 / DEVICE_SYSCLK_FREQ;
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
		return static_cast<uint64_t>(m_period - counter()) * DEVICE_SYSCLK_PERIOD_NS;
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
	const uint64_t m_timeout;
	volatile uint64_t m_start;
public:
	/**
	 * @brief
	 * @param timeout - timeout in milliseconds
	 * @return (none)
	 */
	Timeout(uint64_t timeout = 0)
		: m_timeout(timeout)
		, m_start(SystemClock::now())
	{}

	/**
	 * @brief Check if timeout is expired.
	 * @param (none)
	 * @return \c true if timeout is expired, \c false otherwise.
	 */
	bool expired() volatile
	{
		if (m_timeout == 0)
		{
			return false;
		}
		if ((SystemClock::now() - m_start) > m_timeout)
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
		m_start = SystemClock::now();
	}
};


/// @}
} // namespace chrono


} // namespace mcu


