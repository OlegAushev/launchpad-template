/**
 * @defgroup syslog SysLog
 * 
 * @file syslog.h
 * @ingroup syslog
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once


#include <stdint.h>
#include <stddef.h>
#include "emb/emb_core.h"
#include "emb/emb_queue.h"
#include "mcu_f2837xd/system/mcu_system.h"
#include "mcu_f2837xd/ipc/mcu_ipc.h"

#include "syslog_config.h"


/// @addtogroup syslog
/// @{


/**
 * @brief System logger class.
 */
class SysLog : public emb::monostate<SysLog>
{
public:
	struct IpcFlags
	{
		mcu::ipc::Flag ipcResetErrorsWarnings;
		mcu::ipc::Flag ipcAddMessage;
		mcu::ipc::Flag ipcPopMessage;
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
	SysLog();				// no constructor
	SysLog(const SysLog& other);		// no copy constructor
	SysLog& operator=(const SysLog& other);	// no copy assignment operator

private:
	static emb::Queue<sys::Message::Message, 32> _messages;
#ifdef DUALCORE
	static sys::Message::Message _cpu2Message;
#endif

	static Data _cpu1Data;
#ifdef DUALCORE
	static Data _cpu2Data;
#endif

	static Data* _thisCpuData;

	static IpcFlags _ipcFlags;

public:
	/**
	 * @brief Initializes SysLog.
	 * @param (none)
	 * @return (none)
	 */
	static void init(const IpcFlags& ipcFlags)
	{
		if (initialized())
		{
			return;
		}
		_messages.clear();

#ifdef CPU1
		_thisCpuData = &_cpu1Data;
#endif
#ifdef CPU2
		_thisCpuData = &_cpu2Data;
#endif

		_thisCpuData->errors = 0;
		_thisCpuData->warnings = 0;
		_thisCpuData->enabledErrorMask = 0xFFFFFFFF;
		_thisCpuData->fatalErrorMask = sys::fatalErrors;
		_thisCpuData->fatalWarningMask = sys::fatalWarnings;

		_ipcFlags = ipcFlags;

		_set_initialized();
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
		if (!_messages.full())
		{
			_messages.push(msg);
		}
#else
		if (_ipcFlags.ipcAddMessage.local.check())
		{
			return;
		}
		_cpu2Message = msg;
		_ipcFlags.ipcAddMessage.local.set();
#endif
	}

	/**
	 * @brief Reads front message from message queue.
	 * @param (none)
	 * @return Front message from SysLog message queue.
	 */
	static sys::Message readMessage()
	{
		if (_messages.empty())
		{
			return sys::Message::NoMessage;
		}
		return _messages.front();
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
		if (!_messages.empty())
		{
			_messages.pop();
		}
#else
		_ipcFlags.ipcPopMessage.local.set();
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
		_messages.clear();
	}

	/**
	 * @brief Checks and processes SysLog IPC signals.
	 * @param (none)
	 * @return (none)
	 */
	static void processIpcSignals()
	{
#ifdef DUALCORE
#ifdef CPU1
		if (_ipcFlags.ipcPopMessage.remote.isSet())
		{
			popMessage();
			_ipcFlags.ipcPopMessage.remote.acknowledge();
		}

		if (_ipcFlags.ipcAddMessage.remote.isSet())
		{
			addMessage(_cpu2Message);
			_ipcFlags.ipcAddMessage.remote.acknowledge();
		}
#endif
#ifdef CPU2
		if (_ipcFlags.ipcResetErrorsWarnings.remote.check())
		{
			resetErrorsWarnings();
			_ipcFlags.ipcResetErrorsWarnings.remote.acknowledge();
		}
#endif
#endif
	}

	/**
	 * @brief Enables specified error.
	 * @param error - error to be enabled
	 * @return (none)
	 */
	static void enableError(sys::Error error)
	{
		mcu::CriticalSection cs;
		_thisCpuData->enabledErrorMask = _thisCpuData->enabledErrorMask | (1UL << error.underlying_value());
	}

	/**
	 * @brief Enables all errors.
	 * @param (none)
	 * @return (none)
	 */
	static void enableAllErrors()
	{
		mcu::CriticalSection cs;
		_thisCpuData->enabledErrorMask = 0xFFFFFFFF;
	}

	/**
	 * @brief Disables specified error.
	 * @param error - error to be disabled
	 * @return (none)
	 */
	static void disableError(sys::Error error)
	{
		mcu::CriticalSection cs;
		_thisCpuData->enabledErrorMask = _thisCpuData->enabledErrorMask & ((1UL << error.underlying_value()) ^ 0xFFFFFFFF);
	}

	/**
	 * @brief Disables all errors.
	 * @param (none)
	 * @return (none)
	 */
	static void disableAllErrors()
	{
		mcu::CriticalSection cs;
		_thisCpuData->enabledErrorMask = 0;
	}

	/**
	 * @brief Sets specified error.
	 * @param error  - error to be set
	 * @return (none)
	 */
	static void setError(sys::Error error)
	{
		mcu::CriticalSection cs;
		_thisCpuData->errors = _thisCpuData->errors | ((1UL << error.underlying_value()) & _thisCpuData->enabledErrorMask);
	}

	/**
	 * @brief Checks specified error.
	 * @param error - warning to be checked
	 * @return \c true if error is set, \c false otherwise.
	 */
	static bool hasError(sys::Error error)
	{
#ifdef DUALCORE
		return (_cpu1Data.errors | _cpu2Data.errors) & (1UL << error.underlying_value());
#else
		return _thisCpuData->errors & (1UL << error.underlying_value());
#endif
	}

	/**
	 * @brief Resets specified error.
	 * @param error - error to be reset
	 * @return (none)
	 */
	static void resetError(sys::Error error)
	{
		mcu::CriticalSection cs;
		_thisCpuData->errors = _thisCpuData->errors & ((1UL << error.underlying_value()) ^ 0xFFFFFFFF);
	}

	/**
	 * @brief Returns current system error code.
	 * @param (none)
	 * @return Current system error code.
	 */
	static uint32_t errors()
	{
#ifdef DUALCORE
		return _cpu1Data.errors | _cpu2Data.errors;
#else
		return _thisCpuData->errors;
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
		return (_cpu1Data.errors & _cpu1Data.fatalErrorMask) || (_cpu2Data.errors & _cpu2Data.fatalErrorMask);
#else
		return _thisCpuData->errors & _thisCpuData->fatalErrorMask;
#endif
	}

	/**
	 * @brief Sets specified warning.
	 * @param warning - warning to be set
	 * @return (none)
	 */
	static void setWarning(sys::Warning warning)
	{
		mcu::CriticalSection cs;
		_thisCpuData->warnings = _thisCpuData->warnings | (1UL << warning.underlying_value());
	}

	/**
	 * @brief Checks specified warning.
	 * @param warning - warning to be checked
	 * @return \c true if warning is set, \c false otherwise.
	 */
	static bool hasWarning(sys::Warning warning)
	{
#ifdef DUALCORE
		return (_cpu1Data.warnings | _cpu2Data.warnings) & (1UL << warning.underlying_value());
#else
		return _thisCpuData->warnings & (1UL << warning.underlying_value());
#endif
	}

	/**
	 * @brief Resets specified warning.
	 * @param warning - warning to be reset
	 * @return (none)
	 */
	static void resetWarning(sys::Warning warning)
	{
		mcu::CriticalSection cs;
		_thisCpuData->warnings = _thisCpuData->warnings & ((1UL << warning.underlying_value()) ^ 0xFFFFFFFF);
	}

	/**
	 * @brief Returns current system warning code.
	 * @param (none)
	 * @return Current system warning code.
	 */
	static uint32_t warnings()
	{
#ifdef DUALCORE
		return _cpu1Data.warnings | _cpu2Data.warnings;
#else
		return _thisCpuData->warnings;
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
		_thisCpuData->errors = _thisCpuData->errors & _thisCpuData->fatalErrorMask;
		_thisCpuData->warnings = _thisCpuData->warnings & _thisCpuData->fatalWarningMask;
#if (defined(CPU1) && defined(DUALCORE))
		_ipcFlags.ipcResetErrorsWarnings.local.set();
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
		_thisCpuData->fatalErrorMask = 0;
		_thisCpuData->fatalWarningMask = 0;
	}

	/**
	 * @brief Enables fatal errors and  warnings masks.
	 * @param (none)
	 * @return (none)
	 */
	static void enableCriticalMasks()
	{
		mcu::CriticalSection cs;
		_thisCpuData->fatalErrorMask = sys::fatalErrors;
		_thisCpuData->fatalWarningMask = sys::fatalWarnings;
	}
};


/// @}


