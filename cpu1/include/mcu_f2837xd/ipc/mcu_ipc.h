/**
 * @defgroup mcu_ipc IPC
 * @ingroup mcu
 *
 * @file mcu_ipc.h
 * @ingroup mcu mcu_ipc
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
#include "F2837xD_Ipc_drivers.h"
#include "emb/emb_core.h"


namespace mcu {


namespace ipc {
/// @addtogroup mcu_ipc
/// @{


///
SCOPED_ENUM_DECLARE_BEGIN(Mode)
{
	Singlecore,
	Dualcore
}
SCOPED_ENUM_DECLARE_END(Mode)


///
SCOPED_ENUM_DECLARE_BEGIN(Role)
{
	Primary,
	Secondary
}
SCOPED_ENUM_DECLARE_END(Role)


/**
 * @brief Local IPC flag.
 */
class LocalFlag
{
private:
	uint32_t m_mask;
public:
	LocalFlag() : m_mask(0) {}
	explicit LocalFlag(uint32_t flagNo)
		: m_mask(1UL << flagNo)
	{
		assert(flagNo < 32);
	}

	/**
	 * @brief Sends IPC signal by setting local IPC flag.
	 * @param (none)
	 * @return (none)
	 */
	void set()
	{
		IPCLtoRFlagSet(m_mask);
	}

	/**
	 * @brief Resets local IPC flag.
	 * @param (none)
	 * @return (none)
	 */
	inline void reset()
	{
		IPCLtoRFlagClear(m_mask);
	}

	/**
	 * @brief Checks if local IPC flag is set.
	 * @param (none)
	 * @return \c true if local IPC flag is set, \c false otherwise.
	 */
	bool check()
	{
		return IPCLtoRFlagBusy(m_mask);
	}
};


/**
 * @brief Remote IPC flag.
 */
class RemoteFlag
{
private:
	uint32_t m_mask;
public:
	RemoteFlag() : m_mask(0) {}
	explicit RemoteFlag(uint32_t flagNo)
		: m_mask(1UL << flagNo)
	{
		assert(flagNo < 32);
	}

	/**
	 * @brief Waits for IPC signal in blocking-mode.
	 * @param (none)
	 * @return (none)
	 */
	void wait()
	{
		while(!IPCRtoLFlagBusy(m_mask));
		IPCRtoLFlagAcknowledge(m_mask);
	}

	/**
	 * @brief Checks if remote IPC flag is set.
	 * @param (none)
	 * @return \c true if remote IPC flag is set, \c false otherwise.
	 */
	bool check()
	{
		return IPCRtoLFlagBusy(m_mask);
	}

	/**
	 * @brief Acknowledges remote IPC flag.
	 * @param (none)
	 * @return (none)
	 */
	inline void acknowledge()
	{
		IPCRtoLFlagAcknowledge(m_mask);
	}
};


/**
 * @brief Local-Remote flag "pair".
 */
class Flag
{
private:
	Mode m_mode;
public:
	LocalFlag local;
	RemoteFlag remote;
	Flag() {}
	explicit Flag(uint32_t flagNo, Mode mode)
		: m_mode(mode)
		, local(flagNo)
		, remote(flagNo)
	{}

	/**
	 * @brief Checks if local or remote (according to ipc mode) flag is set.
	 * @param (none)
	 * @return \c true if flag is set, \c false otherwise.
	 */
	bool check()
	{
		switch (m_mode.native_value())
		{
		case mcu::ipc::Mode::Singlecore:
			return local.check();
		case mcu::ipc::Mode::Dualcore:
			return remote.check();
		}
		return false;
	}

	/**
	 * @brief Resets local or remote flag (according to ipc mode).
	 * @param (none)
	 * @return (none)
	 */
	inline void reset()
	{
		switch (m_mode.native_value())
		{
		case mcu::ipc::Mode::Singlecore:
			local.reset();
			return;
		case mcu::ipc::Mode::Dualcore:
			remote.acknowledge();
			return;
		}
	}
};


/// IPC interrupts
enum IpcInterrupt
{
	IpcInterrupt0 = INT_IPC_0,
	IpcInterrupt1 = INT_IPC_1,
	IpcInterrupt2 = INT_IPC_2,
	IpcInterrupt3 = INT_IPC_3,
};


/**
 * @brief Registers IPC ISR.
 * @param ipcInterrupt - IPC interrupt
 * @param func - pointer to ISR
 * @return (none)
 */
inline void registerIpcInterruptHandler(IpcInterrupt ipcInterrupt, void (*handler)(void))
{
	Interrupt_register(ipcInterrupt, handler);
	Interrupt_enable(ipcInterrupt);
}


namespace flags {

extern mcu::ipc::Flag cpu1PeripheryConfigured;
extern mcu::ipc::Flag cpu2Booted;
extern mcu::ipc::Flag cpu2PeripheryConfigured;

}


/// @}
} // namespace ipc


} // namespace mcu


