/**
 * @defgroup mcu_qep QEP
 * @ingroup mcu
 *
 * @file mcu_qep.h
 * @ingroup mcu mcu_qep
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
#include "../gpio/mcu_gpio.h"
#include "emb/emb_core.h"


namespace mcu {


namespace qep {
/// @addtogroup mcu_pwm
/// @{


/// QEP modules
SCOPED_ENUM_DECLARE_BEGIN(Peripheral)
{
	Qep1,
	Qep2,
	Qep3
}
SCOPED_ENUM_DECLARE_END(Peripheral)


/// QEP position counter input mode
SCOPED_ENUM_DECLARE_BEGIN(InputMode)
{
	Quadrature = EQEP_CONFIG_QUADRATURE,
	ClockDir = EQEP_CONFIG_CLOCK_DIR,
	UpCount = EQEP_CONFIG_UP_COUNT,
	DownCount = EQEP_CONFIG_DOWN_COUNT
}
SCOPED_ENUM_DECLARE_END(InputMode)


/// QEP resolution
SCOPED_ENUM_DECLARE_BEGIN(Resolution)
{
	X2 = EQEP_CONFIG_2X_RESOLUTION,
	X1 = EQEP_CONFIG_1X_RESOLUTION
}
SCOPED_ENUM_DECLARE_END(Resolution)


/// QEP swap QEPA and QEPB pins mode
SCOPED_ENUM_DECLARE_BEGIN(SwapAB)
{
	No = EQEP_CONFIG_NO_SWAP,
	Yes = EQEP_CONFIG_SWAP
}
SCOPED_ENUM_DECLARE_END(SwapAB)


/// QEP position counter operating (position reset) mode
SCOPED_ENUM_DECLARE_BEGIN(PositionResetMode)
{
	ResetOnIdx = EQEP_POSITION_RESET_IDX,
	ResetOnMax = EQEP_POSITION_RESET_MAX_POS,
	ResetOn1stIdx = EQEP_POSITION_RESET_1ST_IDX,
	ResetOnTimeout = EQEP_POSITION_RESET_UNIT_TIME_OUT
}
SCOPED_ENUM_DECLARE_END(PositionResetMode)


/**
 * @brief QEP unit config.
 */
struct Configuration
{
	InputMode inputMode;
	Resolution resolution;
	SwapAB swapAB;
	PositionResetMode resetMode;
	uint32_t maxPosition;
	uint32_t timeoutFreq;
	uint32_t latchMode;
	uint16_t initMode;
	uint32_t initPosition;
	EQEP_CAPCLKPrescale capPrescaler;
	EQEP_UPEVNTPrescale eventPrescaler;
	uint16_t intFlags;
};


namespace impl {


/**
 * @brief QEP module implementation.
 */
struct Module
{
	uint32_t base;
	uint16_t intFlags;
	uint32_t pieIntNum;
	Module(uint32_t _base, uint16_t _intFlags, uint32_t _pieIntNum)
		: base(_base), intFlags(_intFlags), pieIntNum(_pieIntNum) {}
};


extern const uint32_t qepBases[3];
extern const uint32_t qepPieIntNums[3];


} // namespace impl


/**
 * @brief QEP unit class.
 */
template <Peripheral::enum_type Instance>
class Module : public emb::c28x::interrupt_invoker<Module<Instance> >, private emb::noncopyable
{
private:
	impl::Module m_module;
public:
	/**
	 * @brief Initializes MCU QEP module.
	 * @param (none)
	 */
	Module(const gpio::Config& qepaPin, const gpio::Config& qepbPin,
			const gpio::Config& qepiPin, const Configuration& conf)
		: emb::c28x::interrupt_invoker<Module<Instance> >(this)
		, m_module(impl::qepBases[Instance], conf.intFlags, impl::qepPieIntNums[Instance])
	{
#ifdef CPU1
		initPins(qepaPin, qepbPin, qepiPin);
#endif

		// Configure the decoder
		EQEP_setDecoderConfig(m_module.base,
				static_cast<uint16_t>(conf.inputMode.underlying_value())
				| static_cast<uint16_t>(conf.resolution.underlying_value())
				| static_cast<uint16_t>(conf.swapAB.underlying_value()));
		EQEP_setEmulationMode(m_module.base, EQEP_EMULATIONMODE_RUNFREE);

		// Configure the position counter to reset on an index event
		EQEP_setPositionCounterConfig(m_module.base,
				static_cast<EQEP_PositionResetMode>(conf.resetMode.underlying_value()), conf.maxPosition);

		// Configure initial position
		EQEP_setPositionInitMode(m_module.base, conf.initMode);
		EQEP_setInitialPosition(m_module.base, conf.initPosition);

		// Enable the unit timer, setting the frequency
		EQEP_enableUnitTimer(m_module.base, (mcu::sysclkFreq() / conf.timeoutFreq));

		// Configure the position counter to be latched on a unit time out
		EQEP_setLatchMode(m_module.base, conf.latchMode);

		// Enable the eQEP1 module
		EQEP_enableModule(m_module.base);

		// Configure and enable the edge-capture unit.
		EQEP_setCaptureConfig(m_module.base, conf.capPrescaler, conf.eventPrescaler);
		EQEP_enableCapture(m_module.base);
	}

	/**
	 * @brief Returns base of QEP-unit.
	 * @param (none)
	 * @return Base of QEP-unit.
	 */
	uint32_t base() const { return m_module.base; }

	/**
	 * @brief Registers interrupt handler.
	 * @param handler - pointer to interrupt handler
	 * @return (none)
	 */
	void registerInterruptHandler(void (*handler)(void))
	{
		Interrupt_register(m_module.pieIntNum, handler);
		EQEP_enableInterrupt(m_module.base, m_module.intFlags);
	}

	/**
	 * @brief Enables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableInterrupts()
	{
		Interrupt_enable(m_module.pieIntNum);
	}

	/**
	 * @brief Disables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableInterrupts()
	{
		Interrupt_disable(m_module.pieIntNum);
	}

protected:
#ifdef CPU1
	static void initPins(const gpio::Config& qepaPin, const gpio::Config& qepbPin,
			const gpio::Config& qepiPin)
	{
		GPIO_setPadConfig(qepaPin.no, GPIO_PIN_TYPE_STD);
		GPIO_setPinConfig(qepaPin.mux);

		if (qepbPin.valid)
		{
			GPIO_setPadConfig(qepbPin.no, GPIO_PIN_TYPE_STD);
			GPIO_setPinConfig(qepbPin.mux);
		}

		if (qepiPin.valid)
		{
			GPIO_setPadConfig(qepiPin.no, GPIO_PIN_TYPE_STD);
			GPIO_setPinConfig(qepiPin.mux);
		}
	}
#endif
};


/// @}
} // namespace qep


} // namespace mcu


