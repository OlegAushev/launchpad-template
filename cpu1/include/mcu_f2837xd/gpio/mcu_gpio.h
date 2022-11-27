/**
 * @defgroup mcu_gpio GPIO
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_gpio
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
	GPIO_CoreSelect masterCore;

	/**
	 * @brief Constructs default GPIO pin config.
	 * @param (none)
	 */
	Config() : valid(false) {}

	/**
	 * @brief Constructs GPIO pin config.
	 * @param _no - pin number
	 * @param _mux - pin mux
	 * @param _direction - pin direction
	 * @param _activeState - pin active state
	 * @param _type - pin type
	 * @param _qualMode - pin qualification mode
	 * @param _qualPeriod - pin qualification period (divider)
	 * @param _masterCore - master core
	 */
	Config(uint32_t _no, uint32_t _mux, Direction _direction, emb::gpio::ActiveState _activeState,
			Type _type, QualMode _qualMode, uint32_t _qualPeriod,
			GPIO_CoreSelect _masterCore = GPIO_CORE_CPU1)
		: valid(true)
		, no(_no)
		, mux(_mux)
		, direction(_direction)
		, activeState(_activeState)
		, type(_type)
		, qualMode(_qualMode)
		, qualPeriod(_qualPeriod)
		, masterCore(_masterCore)
	{}

	/**
	 * @brief Constructs GPIO pin config for muxed pins.
	 * @param _no - pin number
	 * @param _mux - pin mux
	 */
	Config(uint32_t _no, uint32_t _mux)
		: valid(false)
		, no(_no)
		, mux(_mux)
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
	Config m_cfg;
	bool m_initialized;
	GpioBase() : m_initialized(false) {}
public:
	/**
	 * @brief Sets the master core.
	 * @param _masterCore - master core
	 * @return (none)
	 */
	void setMasterCore(GPIO_CoreSelect masterCore)
	{
		assert(m_initialized);
		m_cfg.masterCore = masterCore;
#ifdef CPU1
		GPIO_setMasterCore(m_cfg.no, masterCore);
#endif
	}

	/**
	 * @brief Returns reference to pin config.
	 * @param (none)
	 * @return Reference to pin config.
	 */
	const Config& config() const
	{
		return m_cfg;
	}

	/**
	 * @brief Returns pin number.
	 * @param (none)
	 * @return Pin number.
	 */
	uint32_t no() const
	{
		return m_cfg.no;
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
	GPIO_ExternalIntNum m_intNum;
public:
	/**
	 * @brief GPIO input pin default constructor.
	 * @param (none)
	 */
	Input() {}

	/**
	 * @brief Constructs GPIO input pin.
	 * @param cfg - pin config
	 */
	Input(const Config& cfg)
	{
		init(cfg);
	}

	/**
	 * @brief Initializes GPIO input pin.
	 * @param cfg - pin config
	 * @return (none)
	 */
	void init(const Config& cfg)
	{
		m_cfg = cfg;
		if (m_cfg.valid)
		{
			assert(cfg.direction == Direction::Input);
#ifdef CPU1
			GPIO_setQualificationPeriod(m_cfg.no, m_cfg.qualPeriod);
			GPIO_setQualificationMode(m_cfg.no, static_cast<GPIO_QualificationMode>(m_cfg.qualMode.underlying_value()));
			GPIO_setPadConfig(m_cfg.no, m_cfg.type.underlying_value());
			GPIO_setPinConfig(m_cfg.mux);
			GPIO_setDirectionMode(m_cfg.no, GPIO_DIR_MODE_IN);
			GPIO_setMasterCore(m_cfg.no, m_cfg.masterCore);
#endif
			m_initialized = true;
		}
	}

	/**
	 * @brief Reads pin state.
	 * @param (none)
	 * @return Pin state.
	 */
	virtual emb::gpio::State read() const
	{
		assert(m_initialized);
		return static_cast<emb::gpio::State>(1
				- (GPIO_readPin(m_cfg.no) ^ static_cast<uint32_t>(m_cfg.activeState.underlying_value())));
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
		GPIO_setInterruptPin(m_cfg.no, intNum);	// X-Bar may be configured on CPU1 only
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
		m_intNum = intNum;
		GPIO_setInterruptType(intNum, intType);
		Interrupt_register(impl::pieXIntNums[intNum], handler);
		Interrupt_enable(impl::pieXIntNums[m_intNum]);
	}

	/**
	 * @brief Enables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableInterrupts() const
	{
		GPIO_enableInterrupt(m_intNum);

	}

	/**
	 * @brief Disables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableInterrupts() const
	{
		GPIO_disableInterrupt(m_intNum);
	}

	/**
	 * @brief Acknowledges interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void acknowledgeInterrupt() const
	{
		Interrupt_clearACKGroup(impl::pieXIntGroups[m_intNum]);
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
	 * @param cfg - pin config
	 */
	Output(const Config& cfg)
	{
		init(cfg);
	}

	/**
	 * @brief Initializes GPIO output pin.
	 * @param cfg - pin config
	 * @return (none)
	 */
	void init(const Config& cfg)
	{
		m_cfg = cfg;
		if (m_cfg.valid)
		{
			assert(cfg.direction == Direction::Output);
#ifdef CPU1
			GPIO_setPadConfig(m_cfg.no, m_cfg.type.underlying_value());
			//set() - is virtual, shouldn't be called in ctor
			GPIO_writePin(m_cfg.no, 1
					- (static_cast<uint32_t>(emb::gpio::State::Inactive) ^ static_cast<uint32_t>(m_cfg.activeState.underlying_value())));
			GPIO_setPinConfig(m_cfg.mux);
			GPIO_setDirectionMode(m_cfg.no, GPIO_DIR_MODE_OUT);
			GPIO_setMasterCore(m_cfg.no, m_cfg.masterCore);
#endif
			m_initialized = true;
		}
	}

	/**
	 * @brief Reads pin state.
	 * @param (none)
	 * @return Pin state.
	 */
	virtual emb::gpio::State read() const
	{
		assert(m_initialized);
		return static_cast<emb::gpio::State>(1
				- (GPIO_readPin(m_cfg.no) ^ static_cast<uint32_t>(m_cfg.activeState.underlying_value())));
	}

	/**
	 * @brief Sets pin state.
	 * @param state - pin state
	 * @return (none)
	 */
	virtual void set(emb::gpio::State state = emb::gpio::State::Active) const
	{
		assert(m_initialized);
		GPIO_writePin(m_cfg.no, 1
				- (static_cast<uint32_t>(state.underlying_value()) ^ static_cast<uint32_t>(m_cfg.activeState.underlying_value())));
	}

	/**
	 * @brief Sets pin state to INACTIVE.
	 * @param (none)
	 * @return (none)
	 */
	virtual void reset() const
	{
		assert(m_initialized);
		set(emb::gpio::State::Inactive);
	}

	/**
	 * @brief Toggles pin state.
	 * @param (none)
	 * @return (none)
	 */
	virtual void toggle() const
	{
		assert(m_initialized);
		GPIO_togglePin(m_cfg.no);
	}
};


/*============================================================================*/
/**
 * @brief GPIO input pin debouncing class.
 */
class InputDebouncer
{
private:
	const Input m_pin;
public:
	const unsigned int acqPeriod_ms;
	const unsigned int activeDebounce_ms;
	const unsigned int inactiveDebounce_ms;
private:
	const unsigned int m_activeDebounceCount;
	const unsigned int m_inactiveDebounceCount;
	unsigned int m_count;
	emb::gpio::State m_state;
	bool m_stateChanged;
public:
	/**
	 * @brief Debouncer constructor.
	 * @param pin - GPIO pin
	 * @param acqPeriod_msec - acquisition period (msec)
	 * @param act_msec - time(msec) before registering active state
	 * @param inact_msec - time(msec) before registering inactive state
	 */
	InputDebouncer(const Input& pin, unsigned int acqPeriod_ms, unsigned int act_ms, unsigned int inact_ms)
		: m_pin(pin)
		, acqPeriod_ms(acqPeriod_ms)
		, activeDebounce_ms(act_ms)
		, inactiveDebounce_ms(inact_ms)
		, m_activeDebounceCount(act_ms / acqPeriod_ms)
		, m_inactiveDebounceCount(inact_ms / acqPeriod_ms)
		, m_state(emb::gpio::State::Inactive)
		, m_stateChanged(false)
	{
		m_count = m_activeDebounceCount;
	}

	/**
	 * @brief Service routine called every ACQ_PERIOD_MSEC to debounce both edges.
	 * @param (none)
	 * @return (none)
	 */
	void debounce()
	{
		m_stateChanged = false;
		emb::gpio::State rawState = m_pin.read();

		if (rawState == m_state)
		{
			if (m_state == emb::gpio::State::Active)
			{
				m_count = m_inactiveDebounceCount;
			}
			else
			{
				m_count = m_activeDebounceCount;
			}
		}
		else
		{
			if (--m_count == 0)
			{
				m_state = rawState;
				m_stateChanged = true;
				if (m_state == emb::gpio::State::Active)
				{
					m_count = m_inactiveDebounceCount;
				}
				else
				{
					m_count = m_activeDebounceCount;
				}
			}
		}
	}

	/**
	 * @brief Returns debounced state of pin.
	 * @param (none)
	 * @return Debounced state of pin.
	 */
	emb::gpio::State state() const { return m_state; };

	/**
	 * @brief Checks if state has changed at last debounce() routine run.
	 * @param (none)
	 * @return \c true if state has changed at last debounce(), \c false otherwise.
	 */
	bool stateChanged() const { return m_stateChanged; };
};


/// @}
} //namespace gpio


} // namespace mcu


