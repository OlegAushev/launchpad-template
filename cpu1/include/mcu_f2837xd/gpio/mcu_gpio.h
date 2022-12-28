/**
 * @defgroup mcu_gpio GPIO
 * @ingroup mcu
 *
 * @file mcu_gpio.h
 * @ingroup mcu mcu_gpio
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
#include "emb/emb_interfaces/emb_gpio.h"
#include <assert.h>


namespace mcu {


namespace gpio {
/// @addtogroup mcu_gpio
/// @{


/// Pin types
SCOPED_ENUM_DECLARE_BEGIN(Type)
{
	Std = GPIO_PIN_TYPE_STD,
	Pullup = GPIO_PIN_TYPE_PULLUP,
	Invert = GPIO_PIN_TYPE_INVERT,
	OpenDrain = GPIO_PIN_TYPE_OD
}
SCOPED_ENUM_DECLARE_END(Type)


/// Pin directions
SCOPED_ENUM_DECLARE_BEGIN(Direction)
{
	Input = GPIO_DIR_MODE_IN,
	Output = GPIO_DIR_MODE_OUT
}
SCOPED_ENUM_DECLARE_END(Direction)


/// Pin qualification modes
SCOPED_ENUM_DECLARE_BEGIN(QualMode)
{
	Sync = GPIO_QUAL_SYNC,
	Sample3 = GPIO_QUAL_3SAMPLE,
	Sample6 = GPIO_QUAL_6SAMPLE,
	Async = GPIO_QUAL_ASYNC
}
SCOPED_ENUM_DECLARE_END(QualMode)


/// Pin master core
SCOPED_ENUM_DECLARE_BEGIN(MasterCore)
{
	Cpu1 = GPIO_CORE_CPU1,
	Cpu1Cla1 = GPIO_CORE_CPU1_CLA1,
	Cpu2 = GPIO_CORE_CPU2,
	Cpu2Cla1 = GPIO_CORE_CPU2_CLA1
}
SCOPED_ENUM_DECLARE_END(MasterCore)


namespace impl {


extern const uint32_t pieXIntNums[5];
extern const uint16_t pieXIntGroups[5];


} // namespace impl


/**
 * @brief GPIO pin config.
 */
struct Config
{
	bool valid;
	uint32_t no;
	uint32_t mux;
	Direction direction;
	emb::gpio::ActiveState activeState;
	Type type;
	QualMode qualMode;
	uint32_t qualPeriod;
	MasterCore masterCore;

	/**
	 * @brief Constructs default GPIO pin config.
	 * @param (none)
	 */
	Config() : valid(false) {}

	/**
	 * @brief Constructs GPIO pin config.
	 * @param no_ - pin number
	 * @param mux_ - pin mux
	 * @param direction_ - pin direction
	 * @param activeState_ - pin active state
	 * @param type_ - pin type
	 * @param qualMode_ - pin qualification mode
	 * @param qualPeriod_ - pin qualification period (divider)
	 * @param masterCore_ - master core
	 */
	Config(uint32_t no_, uint32_t mux_, Direction direction_, emb::gpio::ActiveState activeState_,
			Type type_, QualMode qualMode_, uint32_t qualPeriod_,
			MasterCore masterCore_ = MasterCore::Cpu1)
		: valid(true)
		, no(no_)
		, mux(mux_)
		, direction(direction_)
		, activeState(activeState_)
		, type(type_)
		, qualMode(qualMode_)
		, qualPeriod(qualPeriod_)
		, masterCore(masterCore_)
	{}

	/**
	 * @brief Constructs GPIO pin config for muxed pins.
	 * @param no_ - pin number
	 * @param mux_ - pin mux
	 */
	Config(uint32_t no_, uint32_t mux_)
		: valid(false)
		, no(no_)
		, mux(mux_)
	{}

	/**
	 * @brief Constructs config for pin which must not be configured.
	 */
	Config(tag::not_configured) : valid(false) {}
};


/*============================================================================*/
namespace impl {


/**
 * @brief GPIO generic base class.
 */
class GpioBase
{
protected:
	Config _config;
	bool _initialized;
	GpioBase() : _initialized(false) {}
public:
	/**
	 * @brief Sets the master core.
	 * @param _masterCore - master core
	 * @return (none)
	 */
	void setMasterCore(MasterCore masterCore)
	{
		assert(_initialized);
		_config.masterCore = masterCore;
#ifdef CPU1
		GPIO_setMasterCore(_config.no, static_cast<GPIO_CoreSelect>(masterCore.underlying_value()));
#endif
	}

	/**
	 * @brief Returns reference to pin config.
	 * @param (none)
	 * @return Reference to pin config.
	 */
	const Config& config() const
	{
		return _config;
	}

	/**
	 * @brief Returns pin number.
	 * @param (none)
	 * @return Pin number.
	 */
	uint32_t no() const
	{
		return _config.no;
	}
};


} // namespace impl


/*============================================================================*/
/**
 * @brief GPIO input pin class.
 */
class Input : public emb::gpio::IInput, public impl::GpioBase
{
private:
	GPIO_ExternalIntNum _intNum;
public:
	/**
	 * @brief GPIO input pin default constructor.
	 * @param (none)
	 */
	Input() {}

	/**
	 * @brief Constructs GPIO input pin.
	 * @param config - pin config
	 */
	Input(const Config& config)
	{
		init(config);
	}

	/**
	 * @brief Initializes GPIO input pin.
	 * @param config - pin config
	 * @return (none)
	 */
	void init(const Config& config)
	{
		_config = config;
		if (_config.valid)
		{
			assert(config.direction == Direction::Input);
#ifdef CPU1
			GPIO_setQualificationPeriod(_config.no, _config.qualPeriod);
			GPIO_setQualificationMode(_config.no, static_cast<GPIO_QualificationMode>(_config.qualMode.underlying_value()));
			GPIO_setPadConfig(_config.no, _config.type.underlying_value());
			GPIO_setPinConfig(_config.mux);
			GPIO_setDirectionMode(_config.no, GPIO_DIR_MODE_IN);
			GPIO_setMasterCore(_config.no, static_cast<GPIO_CoreSelect>(_config.masterCore.underlying_value()));
#endif
			_initialized = true;
		}
	}

	/**
	 * @brief Reads pin state.
	 * @param (none)
	 * @return Pin state.
	 */
	virtual emb::gpio::State read() const
	{
		assert(_initialized);
		return static_cast<emb::gpio::State>(1
				- (GPIO_readPin(_config.no) ^ static_cast<uint32_t>(_config.activeState.underlying_value())));
	}

public:
	/**
	 * @brief Sets the pin for the specified external interrupt.
	 * @param intNum - interrupt number
	 * @return (none)
	 */
#ifdef CPU1
	void setInterrupt(GPIO_ExternalIntNum intNum)
	{
		GPIO_setInterruptPin(_config.no, intNum);	// X-Bar may be configured on CPU1 only
	}
#endif

	/**
	 * @brief Registers interrupt handler.
	 * @param intNum - interrupt number
	 * @param intType - interrupt type
	 * @param handler - pointer to handler
	 */
	void registerInterruptHandler(GPIO_ExternalIntNum intNum, GPIO_IntType intType, void (*handler)(void))
	{
		_intNum = intNum;
		GPIO_setInterruptType(intNum, intType);
		Interrupt_register(impl::pieXIntNums[intNum], handler);
		Interrupt_enable(impl::pieXIntNums[_intNum]);
	}

	/**
	 * @brief Enables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableInterrupts()
	{
		GPIO_enableInterrupt(_intNum);

	}

	/**
	 * @brief Disables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableInterrupts()
	{
		GPIO_disableInterrupt(_intNum);
	}

	/**
	 * @brief Acknowledges interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void acknowledgeInterrupt()
	{
		Interrupt_clearACKGroup(impl::pieXIntGroups[_intNum]);
	}
};


/*============================================================================*/
/**
 * @brief GPIO output pin class.
 */
class Output : public emb::gpio::IOutput, public impl::GpioBase
{
public:
	/**
	 * @brief GPIO output pin default constructor.
	 * @param (none)
	 */
	Output() {}

	/**
	 * @brief Constructs GPIO output pin.
	 * @param config - pin config
	 */
	Output(const Config& config)
	{
		init(config);
	}

	/**
	 * @brief Initializes GPIO output pin.
	 * @param config - pin config
	 * @return (none)
	 */
	void init(const Config& config)
	{
		_config = config;
		if (_config.valid)
		{
			assert(config.direction == Direction::Output);
#ifdef CPU1
			GPIO_setPadConfig(_config.no, _config.type.underlying_value());
			//set() - is virtual, shouldn't be called in ctor
			GPIO_writePin(_config.no, 1
					- (static_cast<uint32_t>(emb::gpio::State::Inactive) ^ static_cast<uint32_t>(_config.activeState.underlying_value())));
			GPIO_setPinConfig(_config.mux);
			GPIO_setDirectionMode(_config.no, GPIO_DIR_MODE_OUT);
			GPIO_setMasterCore(_config.no, static_cast<GPIO_CoreSelect>(_config.masterCore.underlying_value()));
#endif
			_initialized = true;
		}
	}

	/**
	 * @brief Reads pin state.
	 * @param (none)
	 * @return Pin state.
	 */
	virtual emb::gpio::State read() const
	{
		assert(_initialized);
		return static_cast<emb::gpio::State>(1
				- (GPIO_readPin(_config.no) ^ static_cast<uint32_t>(_config.activeState.underlying_value())));
	}

	/**
	 * @brief Sets pin state.
	 * @param state - pin state
	 * @return (none)
	 */
	virtual void set(emb::gpio::State state = emb::gpio::State::Active)
	{
		assert(_initialized);
		GPIO_writePin(_config.no, 1
				- (static_cast<uint32_t>(state.underlying_value()) ^ static_cast<uint32_t>(_config.activeState.underlying_value())));
	}

	/**
	 * @brief Sets pin state to INACTIVE.
	 * @param (none)
	 * @return (none)
	 */
	virtual void reset()
	{
		assert(_initialized);
		set(emb::gpio::State::Inactive);
	}

	/**
	 * @brief Toggles pin state.
	 * @param (none)
	 * @return (none)
	 */
	virtual void toggle()
	{
		assert(_initialized);
		GPIO_togglePin(_config.no);
	}
};


/*============================================================================*/
/**
 * @brief GPIO input pin debouncing class.
 */
class InputDebouncer
{
private:
	const Input _pin;
public:
	const unsigned int acqPeriod_ms;
	const unsigned int activeDebounce_ms;
	const unsigned int inactiveDebounce_ms;
private:
	const unsigned int _activeDebounceCount;
	const unsigned int _inactiveDebounceCount;
	unsigned int _count;
	emb::gpio::State _state;
	bool _stateChanged;
public:
	/**
	 * @brief Debouncer constructor.
	 * @param pin - GPIO pin
	 * @param acqPeriod_msec - acquisition period (msec)
	 * @param act_msec - time(msec) before registering active state
	 * @param inact_msec - time(msec) before registering inactive state
	 */
	InputDebouncer(const Input& pin, unsigned int acqPeriod_ms, unsigned int act_ms, unsigned int inact_ms)
		: _pin(pin)
		, acqPeriod_ms(acqPeriod_ms)
		, activeDebounce_ms(act_ms)
		, inactiveDebounce_ms(inact_ms)
		, _activeDebounceCount(act_ms / acqPeriod_ms)
		, _inactiveDebounceCount(inact_ms / acqPeriod_ms)
		, _state(emb::gpio::State::Inactive)
		, _stateChanged(false)
	{
		_count = _activeDebounceCount;
	}

	/**
	 * @brief Service routine called every ACQ_PERIOD_MSEC to debounce both edges.
	 * @param (none)
	 * @return (none)
	 */
	void debounce()
	{
		_stateChanged = false;
		emb::gpio::State rawState = _pin.read();

		if (rawState == _state)
		{
			if (_state == emb::gpio::State::Active)
			{
				_count = _inactiveDebounceCount;
			}
			else
			{
				_count = _activeDebounceCount;
			}
		}
		else
		{
			if (--_count == 0)
			{
				_state = rawState;
				_stateChanged = true;
				if (_state == emb::gpio::State::Active)
				{
					_count = _inactiveDebounceCount;
				}
				else
				{
					_count = _activeDebounceCount;
				}
			}
		}
	}

	/**
	 * @brief Returns debounced state of pin.
	 * @param (none)
	 * @return Debounced state of pin.
	 */
	emb::gpio::State state() const { return _state; };

	/**
	 * @brief Checks if state has changed at last debounce() routine run.
	 * @param (none)
	 * @return \c true if state has changed at last debounce(), \c false otherwise.
	 */
	bool stateChanged() const { return _stateChanged; };
};


/*============================================================================*/
/// Duration logger via GPIO mode.
SCOPED_ENUM_DECLARE_BEGIN(DurationLoggerMode)
{
	SetReset,
	Toggle
}
SCOPED_ENUM_DECLARE_END(DurationLoggerMode)


/**
 * @brief
 * @tparam Mode
 */
template <DurationLoggerMode::enum_type Mode = DurationLoggerMode::SetReset>
class DurationLogger
{
private:
	const uint32_t _pin;
public:
	explicit DurationLogger(const mcu::gpio::Output& pin)
		: _pin(pin.no())
	{
		if (Mode == DurationLoggerMode::SetReset)
		{
			GPIO_writePin(_pin, 1);
		}
		else
		{
			GPIO_togglePin(_pin);
			NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;
			GPIO_togglePin(_pin);
		}
	}

	~DurationLogger()
	{
		if (Mode == DurationLoggerMode::SetReset)
		{
			GPIO_writePin(_pin, 0);
		}
		else
		{
			GPIO_togglePin(_pin);
		}
	}
};


/// @}
} //namespace gpio


} // namespace mcu


