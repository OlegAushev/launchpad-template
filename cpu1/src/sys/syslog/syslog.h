/**
 * @defgroup syslog Syslog
 *
 * @file
 * @ingroup syslog
 */


#pragma once


#include <stdint.h>
#include <stddef.h>
#include "emb/emb_core.h"
#include "emb/emb_queue.h"
#include "mcu_f2837xd/system/mcu_system.h"
#include "mcu_f2837xd/ipc/mcu_ipc.h"

#include "syslogconfig.h"


/// @addtogroup syslog
/// @{


/**
 * @brief System logger class.
 */
class Syslog : public emb::monostate<Syslog>
{
public:
	struct IpcFlags
	{
		mcu::ipc::Flag resetErrorsWarnings;
		mcu::ipc::Flag addMessage;
		mcu::ipc::Flag popMessage;
	};

	struct Data
	{
		uint32_t errors;
		uint32_t warnings;
		uint32_t enabledErrorMask;	// enabled errors
		uint32_t fatalErrorMask;	// errors that cannot be reseted by reset()
		uint32_t fatalWarningMask;	// warnings that cannot be reseted by reset()
	};

private:
	Syslog();				// no constructor
	Syslog(const Syslog& other);		// no copy constructor
	Syslog& operator=(const Syslog& other);	// no copy assignment operator

private:
	static emb::Queue<sys::Message::Message, 32> m_messages;
#ifdef DUALCORE
	static sys::Message::Message m_cpu2Message;
#endif

	static Data m_cpu1Data;
#ifdef DUALCORE
	static Data m_cpu2Data;
#endif

	static Data* m_thisCpuData;

	static IpcFlags s_ipcFlags;

public:
	/**
	 * @brief Initializes Syslog.
	 * @param (none)
	 * @return (none)
	 */
	static void init(const IpcFlags& ipcFlags)
	{
		if (initialized())
		{
			return;
		}
		m_messages.clear();

#ifdef CPU1
		m_thisCpuData = &m_cpu1Data;
#endif
#ifdef CPU2
		m_thisCpuData = &m_cpu2Data;
#endif

		m_thisCpuData->errors = 0;
		m_thisCpuData->warnings = 0;
		m_thisCpuData->enabledErrorMask = 0xFFFFFFFF;
		m_thisCpuData->fatalErrorMask = sys::Error::FATAL_ERRORS;
		m_thisCpuData->fatalWarningMask = sys::Warning::FATAL_WARNINGS;

		s_ipcFlags = ipcFlags;

		set_initialized();
	}

	/**
	 * @brief Adds message to message queue or generates IPC signal (CPU2).
	 * @param msg - message to be added
	 * @return (none)
	 */
	static void addMessage(sys::Message::Message msg)
	{
		mcu::CriticalSection cs;
#ifdef CPU1
		if (!m_messages.full())
		{
			m_messages.push(msg);
		}
#else
		if (mcu::isLocalIpcFlagSet(ADD_MESSAGE.local))
		{
			return;
		}
		m_cpu2Message = msg;
		mcu::setLocalIpcFlag(ADD_MESSAGE.local);
#endif
	}

	/**
	 * @brief Reads front message from message queue.
	 * @param (none)
	 * @return Front message from Syslog message queue.
	 */
	static sys::Message::Message readMessage()
	{
		if (m_messages.empty())
		{
			return sys::Message::NO_MESSAGE;
		}
		return m_messages.front();
	}

	/**
	 * @brief Pops message from message queue (CPU1) or generates IPC signal (CPU2).
	 * @param (none)
	 * @return (none)
	 */
	static void popMessage()
	{
		mcu::CriticalSection cs;
#ifdef CPU1
		if (!m_messages.empty())
		{
			m_messages.pop();
		}
#else
		mcu::setLocalIpcFlag(POP_MESSAGE.local);
#endif
	}

	/**
	 * @brief Clears message queue.
	 * @param (none)
	 * @return (none)
	 */
	static void clearMessages()
	{
		mcu::CriticalSection cs;
		m_messages.clear();
	}

	/**
	 * @brief Checks and processes Syslog IPC signals.
	 * @param (none)
	 * @return (none)
	 */
	static void processIpcSignals()
	{
#ifdef DUALCORE
#ifdef CPU1
		if (s_ipcFlags.popMessage.remote.check())
		{
			popMessage();
			s_ipcFlags.popMessage.remote.acknowledge();
		}

		if (s_ipcFlags.addMessage.remote.check())
		{
			addMessage(m_cpu2Message);
			s_ipcFlags.addMessage.remote.acknowledge();
		}
#endif
#ifdef CPU2
		if (s_ipcFlags.resetErrorsWarnings.remote.check())
		{
			resetErrorsWarnings();
			s_ipcFlags.resetErrorsWarnings.remote.acknowledge();
		}
#endif
#endif
	}

	/**
	 * @brief Enables specified error.
	 * @param error - error to be enabled
	 * @return (none)
	 */
	static void enableError(sys::Error::Error error)
	{
		mcu::CriticalSection cs;
		m_thisCpuData->enabledErrorMask = m_thisCpuData->enabledErrorMask | (1UL << error);
	}

	/**
	 * @brief Enables all errors.
	 * @param (none)
	 * @return (none)
	 */
	static void enableAllErrors()
	{
		mcu::CriticalSection cs;
		m_thisCpuData->enabledErrorMask = 0xFFFFFFFF;
	}

	/**
	 * @brief Disables specified error.
	 * @param error - error to be disabled
	 * @return (none)
	 */
	static void disableError(sys::Error::Error error)
	{
		mcu::CriticalSection cs;
		m_thisCpuData->enabledErrorMask = m_thisCpuData->enabledErrorMask & ((1UL << error) ^ 0xFFFFFFFF);
	}

	/**
	 * @brief Disables all errors.
	 * @param (none)
	 * @return (none)
	 */
	static void disableAllErrors()
	{
		mcu::CriticalSection cs;
		m_thisCpuData->enabledErrorMask = 0;
	}

	/**
	 * @brief Sets specified error.
	 * @param error  - error to be set
	 * @return (none)
	 */
	static void setError(sys::Error::Error error)
	{
		mcu::CriticalSection cs;
		m_thisCpuData->errors = m_thisCpuData->errors | ((1UL << error) & m_thisCpuData->enabledErrorMask);
	}

	/**
	 * @brief Checks specified error.
	 * @param error - warning to be checked
	 * @return \c true if error is set, \c false otherwise.
	 */
	static bool hasError(sys::Error::Error error)
	{
#ifdef DUALCORE
		return (m_cpu1Data.errors | m_cpu2Data.errors) & (1UL << error);
#else
		return m_thisCpuData->errors & (1UL << error);
#endif
	}

	/**
	 * @brief Resets specified error.
	 * @param error - error to be reset
	 * @return (none)
	 */
	static void resetError(sys::Error::Error error)
	{
		mcu::CriticalSection cs;
		m_thisCpuData->errors = m_thisCpuData->errors & ((1UL << error) ^ 0xFFFFFFFF);
	}

	/**
	 * @brief Returns current system error code.
	 * @param (none)
	 * @return Current system error code.
	 */
	static uint32_t errors()
	{
#ifdef DUALCORE
		return m_cpu1Data.errors | m_cpu2Data.errors;
#else
		return m_thisCpuData->errors;
#endif
	}

	/**
	 * @brief Checks if system has fatal errors.
	 * @param (none)
	 * @return True if system has fatal errors.
	 */
	static bool hasFatalErrors()
	{
#ifdef DUALCORE
		return (m_cpu1Data.errors & m_cpu1Data.fatalErrorMask) || (m_cpu2Data.errors & m_cpu2Data.fatalErrorMask);
#else
		return m_thisCpuData->errors & m_thisCpuData->fatalErrorMask;
#endif
	}

	/**
	 * @brief Sets specified warning.
	 * @param warning - warning to be set
	 * @return (none)
	 */
	static void setWarning(sys::Warning::Warning warning)
	{
		mcu::CriticalSection cs;
		m_thisCpuData->warnings = m_thisCpuData->warnings | (1UL << warning);
	}

	/**
	 * @brief Checks specified warning.
	 * @param warning - warning to be checked
	 * @return \c true if warning is set, \c false otherwise.
	 */
	static bool hasWarning(sys::Warning::Warning warning)
	{
#ifdef DUALCORE
		return (m_cpu1Data.warnings | m_cpu2Data.warnings) & (1UL << warning);
#else
		return m_thisCpuData->warnings & (1UL << warning);
#endif
	}

	/**
	 * @brief Resets specified warning.
	 * @param warning - warning to be reset
	 * @return (none)
	 */
	static void resetWarning(sys::Warning::Warning warning)
	{
		mcu::CriticalSection cs;
		m_thisCpuData->warnings = m_thisCpuData->warnings & ((1UL << warning) ^ 0xFFFFFFFF);
	}

	/**
	 * @brief Returns current system warning code.
	 * @param (none)
	 * @return Current system warning code.
	 */
	static uint32_t warnings()
	{
#ifdef DUALCORE
		return m_cpu1Data.warnings | m_cpu2Data.warnings;
#else
		return m_thisCpuData->warnings;
#endif
	}

	/**
	 * @brief Resets non-fatal errors and warnings.
	 * @param tag - soft reset tag
	 * @return (none)
	 */
	static void resetErrorsWarnings()
	{
		mcu::CriticalSection cs;
		m_thisCpuData->errors = m_thisCpuData->errors & m_thisCpuData->fatalErrorMask;
		m_thisCpuData->warnings = m_thisCpuData->warnings & m_thisCpuData->fatalWarningMask;
#if (defined(CPU1) && defined(DUALCORE))
		s_ipcFlags.resetErrorsWarnings.local.set();
#endif
	}

	/**
	 * @brief Disables fatal errors and warnings masks.
	 * @param (none)
	 * @return (none)
	 */
	static void clearCriticalMasks()
	{
		mcu::CriticalSection cs;
		m_thisCpuData->fatalErrorMask = 0;
		m_thisCpuData->fatalWarningMask = 0;
	}

	/**
	 * @brief Enables fatal errors and  warnings masks.
	 * @param (none)
	 * @return (none)
	 */
	static void enableCriticalMasks()
	{
		mcu::CriticalSection cs;
		m_thisCpuData->fatalErrorMask = sys::Error::FATAL_ERRORS;
		m_thisCpuData->fatalWarningMask = sys::Warning::FATAL_WARNINGS;
	}
};


/// @}


