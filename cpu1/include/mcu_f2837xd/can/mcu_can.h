/**
 * @defgroup mcu_can CAN
 * @ingroup mcu
 * 
 * @file mcu_can.h
 * @ingroup mcu mcu_can
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
#include "../gpio/mcu_gpio.h"
#include "emb/emb_core.h"


namespace mcu {


namespace can {
/// @addtogroup mcu_can
/// @{


/// CAN modules
SCOPED_ENUM_DECLARE_BEGIN(Peripheral)
{
	CanA,
	CanB
}
SCOPED_ENUM_DECLARE_END(Peripheral)


/// CAN bitrates
SCOPED_ENUM_UT_DECLARE_BEGIN(Bitrate, uint32_t)
{
	Bitrate125K = 125000,
	Bitrate500K = 500000,
	Bitrate1M = 1000000,
}
SCOPED_ENUM_DECLARE_END(Bitrate)


///
SCOPED_ENUM_DECLARE_BEGIN(Mode)
{
	Normal = 0,
	Silent = CAN_TEST_SILENT,
	Loopback = CAN_TEST_LBACK,
	ExLoopback = CAN_TEST_EXL,
	SilentLoopback = CAN_TEST_SILENT | CAN_TEST_LBACK
}
SCOPED_ENUM_DECLARE_END(Mode)


/**
 * @brief CAN message object.
 */
struct MessageObject
{
	uint32_t objId;
	uint32_t frameId;
	CAN_MsgFrameType frameType;
	CAN_MsgObjType objType;
	uint32_t frameIdMask;
	uint32_t flags;
	uint16_t dataLen;
};


namespace impl {


/**
 * @brief CAN module implementation.
 */
struct Module
{
	uint32_t base;
	uint32_t pieIntNum;
	Module(uint32_t _base, uint32_t _pieIntNum)
		: base(_base), pieIntNum(_pieIntNum) {}
};


extern const uint32_t canBases[2];
extern const uint32_t canPieIntNums[2];


} // namespace impl


/**
 * @brief CAN module class.
 */
template <Peripheral::enum_type Instance>
class Module : public emb::c28x::interrupt_invoker<Module<Instance> >, private emb::noncopyable
{
private:
	impl::Module _module;
public:
	/**
	 * @brief Initializes MCU CAN module.
	 * @param rxPin	- MCU CAN-RX pin config
	 * @param txPin - MCU CAN-TX pin config
	 * @param bitrate - CAN bus bitrate
	 * @param mode - CAN mode
	 */
	Module(const gpio::Config& rxPin, const gpio::Config& txPin,
			Bitrate bitrate, Mode mode)
		: emb::c28x::interrupt_invoker<Module<Instance> >(this)
		, _module(impl::canBases[Instance], impl::canPieIntNums[Instance])
	{
#ifdef CPU1
		_initPins(rxPin, txPin);
#endif

		CAN_initModule(_module.base);
		CAN_selectClockSource(_module.base, CAN_CLOCK_SOURCE_SYS);

		switch (bitrate.native_value())
		{
		case Bitrate::Bitrate125K:
		case Bitrate::Bitrate500K:
			CAN_setBitRate(_module.base, mcu::sysclkFreq(), static_cast<uint32_t>(bitrate.underlying_value()), 16);
			break;
		case Bitrate::Bitrate1M:
			CAN_setBitRate(_module.base, mcu::sysclkFreq(), static_cast<uint32_t>(bitrate.underlying_value()), 10);
			break;
		}

		CAN_setAutoBusOnTime(_module.base, 0);
		CAN_enableAutoBusOn(_module.base);

		if (mode != Mode::Normal)
		{
			CAN_enableTestMode(_module.base, static_cast<uint16_t>(mode.underlying_value()));
		}

		CAN_startModule(_module.base);
	}

#ifdef CPU1
	/**
	 * @brief Transfers control over CAN unit to CPU2.
	 * @param rxPin - MCU CAN-RX pin config
	 * @param txPin - MCU CAN-TX pin config
	 * @return (none)
	 */
	static void transferControlToCpu2(const gpio::Config& rxPin, const gpio::Config& txPin)
	{
		_initPins(rxPin, txPin);
		GPIO_setMasterCore(rxPin.no, GPIO_CORE_CPU2);
		GPIO_setMasterCore(txPin.no, GPIO_CORE_CPU2);

		SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL8_CAN,
				static_cast<uint16_t>(Instance)+1, SYSCTL_CPUSEL_CPU2);
	}
#endif

	/**
	 * @brief Returns base of CAN-unit.
	 * @param (none)
	 * @return Base of CAN-unit.
	 */
	uint32_t base() const { return _module.base; }

	/**
	 * @brief Retrieves received data.
	 * @param objId - message object ID
	 * @param dataBuf - pointer to data destination buffer
	 * @return \c true if new data was retrieved, \c false otherwise.
	 */
	bool recv(uint32_t objId, uint16_t* dataBuf)
	{
		return CAN_readMessage(_module.base, objId, dataBuf);
	}

	/**
	 * @brief Sends data.
	 * @param objId - message object ID
	 * @param dataBuf - pointer to data source buffer
	 * @param dataLen - data length
	 * @return (none)
	 */
	void send(uint32_t objId, const uint16_t* dataBuf, uint16_t dataLen)
	{
		CAN_sendMessage(_module.base, objId, dataLen, dataBuf);
	}

	/**
	 * @brief Setups message object.
	 * @param msgObj - message object
	 * @return (none)
	 */
	void setupMessageObject(MessageObject& msgObj)
	{
		CAN_setupMessageObject(_module.base, msgObj.objId, msgObj.frameId, msgObj.frameType,
				msgObj.objType, msgObj.frameIdMask, msgObj.flags, msgObj.dataLen);
	}

	/**
	 * @brief Registers interrupt handler.
	 * @param handler - pointer to interrupt handler
	 * @return (none)
	 */
	void registerInterruptHandler(void (*handler)(void))
	{
		Interrupt_register(_module.pieIntNum, handler);
		CAN_enableInterrupt(_module.base, CAN_INT_IE0 | CAN_INT_ERROR | CAN_INT_STATUS);
		CAN_enableGlobalInterrupt(_module.base, CAN_GLOBAL_INT_CANINT0);
	}

	/**
	 * @brief Registers interrupt callback.
	 * @param callback - pointer to interrupt callback
	 * @return (none)
	 */
	void registerInterruptCallback(void (*callback)(Module*, uint32_t, uint16_t))
	{
		registerInterruptHandler(onInterrupt);
		_onInterruptCallback = callback;
	}

	/**
	 * @brief Enables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableInterrupts() { Interrupt_enable(_module.pieIntNum); }

	/**
	 * @brief Disables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableInterrupts() { Interrupt_disable(_module.pieIntNum); }

	/**
	 * @brief Acknowledges interrupt.
	 * @param intCause - interrupt cause
	 * @return (none)
	 */
	void acknowledgeInterrupt(uint32_t intCause)
	{
		CAN_clearInterruptStatus(_module.base, intCause);
		CAN_clearGlobalInterruptStatus(_module.base, CAN_GLOBAL_INT_CANINT0);
		Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
	}

protected:
#ifdef CPU1
	static void _initPins(const gpio::Config& rxPin, const gpio::Config& txPin)
	{
		GPIO_setPinConfig(rxPin.mux);
		GPIO_setPinConfig(txPin.mux);
	}
#endif

	static void (*_onInterruptCallback)(Module*, uint32_t, uint16_t);

	static interrupt void onInterrupt()
	{
		uint32_t interruptCause = CAN_getInterruptCause(Module<Instance>::instance()->base());
		uint16_t status = CAN_getStatus(Module<Instance>::instance()->base());

		_onInterruptCallback(Module<Instance>::instance(), interruptCause, status);

		Module<Instance>::instance()->acknowledgeInterrupt(interruptCause);
	}
};


template <Peripheral::enum_type Instance>
void (*Module<Instance>::_onInterruptCallback)(Module*, uint32_t, uint16_t);


/// @}
} // namespace can


} // namespace mcu


