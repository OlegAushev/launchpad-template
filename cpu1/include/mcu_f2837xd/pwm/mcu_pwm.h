/**
 * @defgroup mcu_pwm PWM
 * @ingroup mcu
 *
 * @file mcu_pwm.h
 * @ingroup mcu mcu_pwm
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
#include "emb/emb_array.h"
#include <math.h>


namespace mcu {


namespace pwm {
/// @addtogroup mcu_pwm
/// @{


/// PWM states
SCOPED_ENUM_DECLARE_BEGIN(State)
{
	Off,
	On
}
SCOPED_ENUM_DECLARE_END(State)


/// PWM modules
SCOPED_ENUM_DECLARE_BEGIN(Peripheral)
{
	Pwm1,
	Pwm2,
	Pwm3,
	Pwm4,
	Pwm5,
	Pwm6,
	Pwm7,
	Pwm8,
	Pwm9,
	Pwm10,
	Pwm11,
	Pwm12
}
SCOPED_ENUM_DECLARE_END(Peripheral)


/// PWM phases count
SCOPED_ENUM_DECLARE_BEGIN(PhaseCount)
{
	One = 1,
	Three = 3,
	Six = 6
}
SCOPED_ENUM_DECLARE_END(PhaseCount)


/// PWM clock divider
SCOPED_ENUM_DECLARE_BEGIN(ClockDivider)
{
	Divider1 = EPWM_CLOCK_DIVIDER_1,
	Divider2 = EPWM_CLOCK_DIVIDER_2,
	Divider4 = EPWM_CLOCK_DIVIDER_4,
	Divider8 = EPWM_CLOCK_DIVIDER_8,
	Divider16 = EPWM_CLOCK_DIVIDER_16,
	Divider32 = EPWM_CLOCK_DIVIDER_32,
	Divider64 = EPWM_CLOCK_DIVIDER_64,
	Divider128 = EPWM_CLOCK_DIVIDER_128
}
SCOPED_ENUM_DECLARE_END(ClockDivider)


/// PWM hs clock divider
SCOPED_ENUM_DECLARE_BEGIN(HsClockDivider)
{
	Divider1 = EPWM_HSCLOCK_DIVIDER_1,
	Divider2 = EPWM_HSCLOCK_DIVIDER_2,
	Divider4 = EPWM_HSCLOCK_DIVIDER_4,
	Divider6 = EPWM_HSCLOCK_DIVIDER_6,
	Divider8 = EPWM_HSCLOCK_DIVIDER_8,
	Divider10 = EPWM_HSCLOCK_DIVIDER_10,
	Divider12 = EPWM_HSCLOCK_DIVIDER_12,
	Divider14 = EPWM_HSCLOCK_DIVIDER_14
}
SCOPED_ENUM_DECLARE_END(HsClockDivider)


/// PWM mode (waveform)
SCOPED_ENUM_DECLARE_BEGIN(OperatingMode)
{
	ActiveHighComplementary,
	ActiveLowComplementary
}
SCOPED_ENUM_DECLARE_END(OperatingMode)


/// PWM counter mode
SCOPED_ENUM_DECLARE_BEGIN(CounterMode)
{
	Up = EPWM_COUNTER_MODE_UP,
	Down = EPWM_COUNTER_MODE_DOWN,
	UpDown = EPWM_COUNTER_MODE_UP_DOWN
}
SCOPED_ENUM_DECLARE_END(CounterMode)


/// PWM outputs swap
SCOPED_ENUM_DECLARE_BEGIN(OutputSwap)
{
	No,
	Yes
}
SCOPED_ENUM_DECLARE_END(OutputSwap)


/**
 * @brief PWM config.
 */
template <PhaseCount::enum_type PhaseCount>
struct Config
{
	Peripheral peripheral[PhaseCount];
	float switchingFreq;
	float deadtime_ns;
	uint32_t clockPrescaler;	// must be the product of clkDivider and hsclkDivider
	ClockDivider clkDivider;
	HsClockDivider hsclkDivider;
	OperatingMode operatingMode;
	CounterMode counterMode;
	OutputSwap outputSwap;
	uint16_t eventInterruptSource;
	bool adcTriggerEnable[2];
	EPWM_ADCStartOfConversionSource adcTriggerSource[2];
};


namespace impl {


/**
 * @brief PWM module implementation.
 */
template <PhaseCount::enum_type PhaseCount>
struct Module
{
	Peripheral instance[PhaseCount];
	uint32_t base[PhaseCount];
	uint32_t pieEventIntNum;
	uint32_t pieTripIntNum;
};


extern const uint32_t pwmBases[12];
extern const uint32_t pwmPieEventIntNums[12];
extern const uint32_t pwmPieTripIntNums[12];
extern const uint32_t pwmPinOutAConfigs[12];
extern const uint32_t pwmPinOutBConfigs[12];


} // namespace impl


/**
 * @brief PWM unit class.
 */
template <PhaseCount::enum_type Phases>
class Module : private emb::noncopyable
{
private:
	// there is a divider ( EPWMCLKDIV ) of the system clock
	// which defaults to EPWMCLK = SYSCLKOUT/2, fclk(epwm)max = 100 MHz
	static const uint32_t _pwmClkFreq = DEVICE_SYSCLK_FREQ / 2;
	static const uint32_t _pwmClkCycle_ns = 1000000000 / _pwmClkFreq;
	const uint32_t _timebaseClkFreq;
	const uint32_t _timebaseCycle_ns;

	impl::Module<Phases> _module;
	CounterMode _counterMode;
	float _switchingFreq;
	uint16_t _deadtimeCycles;

	uint16_t _period;		// TBPRD register value
	uint16_t _phaseShift[Phases];	// TBPHS registers values

	State _state;
public:
	/**
	 * @brief Initializes MCU PWM module.
	 * @param config - PWM config
	 * @param (none)
	 */
	Module(const pwm::Config<Phases>& config)
		: _timebaseClkFreq(_pwmClkFreq / config.clockPrescaler)
		, _timebaseCycle_ns(_pwmClkCycle_ns * config.clockPrescaler)
		, _counterMode(config.counterMode)
		, _switchingFreq(config.switchingFreq)
		, _deadtimeCycles(config.deadtime_ns / _timebaseCycle_ns)
		, _state(State::Off)
	{
		for (size_t i = 0; i < Phases; ++i)
		{
			_module.instance[i] = config.peripheral[i];
			_module.base[i] = impl::pwmBases[config.peripheral[i].underlying_value()];
		}
		_module.pieEventIntNum = impl::pwmPieEventIntNums[config.peripheral[0].underlying_value()];
		_module.pieTripIntNum = impl::pwmPieTripIntNums[config.peripheral[0].underlying_value()];

		for (size_t i = 0; i < Phases; ++i)
		{
			_phaseShift[i] = 0;
		}

#ifdef CPU1
		initPins(config);
#else
		EMB_UNUSED(impl::pwmPinOutAConfigs);
		EMB_UNUSED(impl::pwmPinOutBConfigs);
#endif

		// Disable sync, freeze clock to PWM
		SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

		/* ========================================================================== */
		// Calculate TBPRD value
		switch (config.counterMode.native_value())
		{
		case CounterMode::Up:
		case CounterMode::Down:
			_period = (_timebaseClkFreq / _switchingFreq) - 1;
			break;
		case CounterMode::UpDown:
			_period = (_timebaseClkFreq / _switchingFreq) / 2;
			break;
		}

		for (size_t i = 0; i < Phases; ++i)
		{
			EPWM_setTimeBasePeriod(_module.base[i], _period);
			EPWM_setTimeBaseCounter(_module.base[i], 0);

			/* ========================================================================== */
			// Clock prescaler
			EPWM_setClockPrescaler(_module.base[i],
					static_cast<EPWM_ClockDivider>(config.clkDivider.underlying_value()),
					static_cast<EPWM_HSClockDivider>(config.hsclkDivider.underlying_value()));

			/* ========================================================================== */
			// Compare values
			EPWM_setCounterCompareValue(_module.base[i], EPWM_COUNTER_COMPARE_A, 0);

			/* ========================================================================== */
			// Counter mode
			EPWM_setTimeBaseCounterMode(_module.base[i],
					static_cast<EPWM_TimeBaseCountMode>(config.counterMode.underlying_value()));

#ifdef CPU1
			/* ========================================================================== */
			// Sync input source for the EPWM signals
			switch (_module.base[i])
			{
			case EPWM4_BASE:
				SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM4, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
				break;
			case EPWM7_BASE:
				SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM7, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
				break;
			case EPWM10_BASE:
				SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM10, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
				break;
			default:
				break;
			}
#endif

			/* ========================================================================== */
			// Sync out pulse event
			switch (Phases)
			{
			case PhaseCount::Six:
			case PhaseCount::Three:
				if ((i == 0) && (_module.base[i] == EPWM1_BASE))
				{
					// EPWM1 is master
					EPWM_setSyncOutPulseMode(_module.base[i], EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO);
				}
				else
				{
					// other modules sync is pass-through
					EPWM_setSyncOutPulseMode(_module.base[i], EPWM_SYNC_OUT_PULSE_ON_EPWMxSYNCIN);
				}
				break;
			case PhaseCount::One:
				if (_module.base[i] == EPWM1_BASE)
				{
					EPWM_setSyncOutPulseMode(_module.base[i], EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO);
				}
				else
				{
					EPWM_setSyncOutPulseMode(_module.base[i], EPWM_SYNC_OUT_PULSE_ON_EPWMxSYNCIN);
				}
				break;
			}

			/* ========================================================================== */
			// Time-base counter synchronization and phase shift
			switch (Phases)
			{
			case PhaseCount::Six:
			case PhaseCount::Three:
				if ((i == 0) && (_module.base[i] == EPWM1_BASE))
				{
					// EPWM1 is master, EPWM4,7,10 are synced to it
					// master has no phase shift
					EPWM_disablePhaseShiftLoad(_module.base[i]);
					EPWM_setPhaseShift(_module.base[i], 0);
				}
				else
				{
					EPWM_enablePhaseShiftLoad(_module.base[i]);
					// start counting up when sync occurs
					// used when the time-base counter is configured in the up-down-count mode
					EPWM_setCountModeAfterSync(_module.base[i], EPWM_COUNT_MODE_UP_AFTER_SYNC);
					// 2 x EPWMCLK - delay from internal master module to slave modules, p.1876
					EPWM_setPhaseShift(_module.base[i], 2 + _phaseShift[i]);
				}
				break;

			case PhaseCount::One:
				if (_module.base[i] == EPWM1_BASE)
				{
					EPWM_disablePhaseShiftLoad(_module.base[i]);
					EPWM_setPhaseShift(_module.base[i], 0);
				}
				else
				{
					EPWM_enablePhaseShiftLoad(_module.base[i]);
					EPWM_setCountModeAfterSync(_module.base[i], EPWM_COUNT_MODE_UP_AFTER_SYNC);
					EPWM_setPhaseShift(_module.base[i], 2 + _phaseShift[i]);
				}
				break;
			}

			/* ========================================================================== */
			// Shadowing
			EPWM_selectPeriodLoadEvent(_module.base[i], EPWM_SHADOW_LOAD_MODE_COUNTER_ZERO);
			EPWM_setCounterCompareShadowLoadMode(_module.base[i], EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);
			EPWM_setActionQualifierContSWForceShadowMode(_module.base[i], EPWM_AQ_SW_IMMEDIATE_LOAD);

			/* ========================================================================== */
			// CMPA actions
				// PWMxA configuration for typical waveforms
			switch (config.operatingMode.native_value())
			{
			case CounterMode::Up:
				EPWM_setActionQualifierAction(_module.base[i],	EPWM_AQ_OUTPUT_A,
						EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
				EPWM_setActionQualifierAction(_module.base[i], EPWM_AQ_OUTPUT_A,
						EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
				break;
			case CounterMode::Down:
				EPWM_setActionQualifierAction(_module.base[i],	EPWM_AQ_OUTPUT_A,
						EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
				EPWM_setActionQualifierAction(_module.base[i], EPWM_AQ_OUTPUT_A,
						EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);
				break;
			case CounterMode::UpDown:
				EPWM_setActionQualifierAction(_module.base[i], EPWM_AQ_OUTPUT_A,
						EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
				EPWM_setActionQualifierAction(_module.base[i], EPWM_AQ_OUTPUT_A,
						EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
				break;
			}
				// PWMxB configuration - always LOW: typically only PWMxA is used by dead-band submodule
			EPWM_setActionQualifierAction(_module.base[i],	EPWM_AQ_OUTPUT_B,
					EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
			EPWM_setActionQualifierAction(_module.base[i], EPWM_AQ_OUTPUT_B,
					EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
			EPWM_setActionQualifierAction(_module.base[i], EPWM_AQ_OUTPUT_B,
					EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
			EPWM_setActionQualifierAction(_module.base[i], EPWM_AQ_OUTPUT_B,
					EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);

			/* ========================================================================== */
			// Dead-Band
			EPWM_setDeadBandControlShadowLoadMode(_module.base[i], EPWM_DB_LOAD_ON_CNTR_ZERO);
			EPWM_setDeadBandDelayMode(_module.base[i], EPWM_DB_FED, true);
			EPWM_setDeadBandDelayMode(_module.base[i], EPWM_DB_RED, true);

			switch (config.operatingMode.native_value())
			{
			case OperatingMode::ActiveHighComplementary:
				EPWM_setDeadBandDelayPolarity(_module.base[i], EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_HIGH);
				EPWM_setDeadBandDelayPolarity(_module.base[i], EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW);
				break;
			case OperatingMode::ActiveLowComplementary:
				EPWM_setDeadBandDelayPolarity(_module.base[i], EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_LOW);
				EPWM_setDeadBandDelayPolarity(_module.base[i], EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_HIGH);
				break;
			}

			EPWM_setRisingEdgeDeadBandDelayInput(_module.base[i], EPWM_DB_INPUT_EPWMA);
			EPWM_setFallingEdgeDeadBandDelayInput(_module.base[i], EPWM_DB_INPUT_EPWMA);
			EPWM_setRisingEdgeDelayCount(_module.base[i], _deadtimeCycles);
			EPWM_setFallingEdgeDelayCount(_module.base[i], _deadtimeCycles);
			EPWM_setDeadBandCounterClock(_module.base[i], EPWM_DB_COUNTER_CLOCK_FULL_CYCLE);

			switch (config.outputSwap.native_value())
			{
			case OutputSwap::No:
				EPWM_setDeadBandOutputSwapMode(_module.base[i], EPWM_DB_OUTPUT_A, false);
				EPWM_setDeadBandOutputSwapMode(_module.base[i], EPWM_DB_OUTPUT_B, false);
				break;
			case OutputSwap::Yes:
				EPWM_setDeadBandOutputSwapMode(_module.base[i], EPWM_DB_OUTPUT_A, true);
				EPWM_setDeadBandOutputSwapMode(_module.base[i], EPWM_DB_OUTPUT_B, true);
				break;
			}

			/* ========================================================================== */
			// Trip-Zone actions
			switch (config.operatingMode.native_value())
			{
			case OperatingMode::ActiveHighComplementary:
				EPWM_setTripZoneAction(_module.base[i], EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
				EPWM_setTripZoneAction(_module.base[i], EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_LOW);
				break;
			case OperatingMode::ActiveLowComplementary:
				EPWM_setTripZoneAction(_module.base[i], EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_HIGH);
				EPWM_setTripZoneAction(_module.base[i], EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_HIGH);
				break;
			}

			EPWM_clearOneShotTripZoneFlag(_module.base[i], EPWM_TZ_OST_FLAG_OST1);
			EPWM_clearTripZoneFlag(_module.base[i], EPWM_TZ_INTERRUPT | EPWM_TZ_FLAG_OST);
		}

		/* ========================================================================== */
		// ADC Trigger configuration, only first module triggers ADC
		if (config.adcTriggerEnable[0])
		{
			EPWM_setADCTriggerSource(_module.base[0], EPWM_SOC_A, config.adcTriggerSource[0]);
			EPWM_setADCTriggerEventPrescale(_module.base[0], EPWM_SOC_A, 1);
			EPWM_enableADCTrigger(_module.base[0], EPWM_SOC_A);
		}

		if (config.adcTriggerEnable[1])
		{
			EPWM_setADCTriggerSource(_module.base[0], EPWM_SOC_B, config.adcTriggerSource[1]);
			EPWM_setADCTriggerEventPrescale(_module.base[0], EPWM_SOC_B, 1);
			EPWM_enableADCTrigger(_module.base[0], EPWM_SOC_B);
		}

		/* ========================================================================== */
		// Interrupts, only interrupt on first module is required
		EPWM_setInterruptSource(_module.base[0], config.eventInterruptSource);
		EPWM_setInterruptEventCount(_module.base[0], 1U);

		_initCustomOptions();

		stop();

		// Enable sync and clock to PWM
		SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
	}

#ifdef CPU1
	/**
	 * @brief Initializes PWM trip-zone submodule.
	 * @param pin - trip input
	 * @return (none)
	 */
	void initTzSubmodule(const gpio::Input& pin, XBAR_InputNum xbarInput)
	{
		assert(static_cast<uint32_t>(xbarInput) <= static_cast<uint32_t>(XBAR_INPUT3));

		switch (pin.config().activeState)
		{
		case emb::ActiveLow:
			GPIO_setPadConfig(pin.config().no, GPIO_PIN_TYPE_PULLUP);
			break;
		case emb::ActiveHigh:
			GPIO_setPadConfig(pin.config().no, GPIO_PIN_TYPE_INVERT);
			break;
		}

		GPIO_setPinConfig(pin.config().mux);
		GPIO_setDirectionMode(pin.config().no, GPIO_DIR_MODE_IN);
		GPIO_setQualificationMode(pin.config().no, GPIO_QUAL_ASYNC);

		XBAR_setInputPin(xbarInput, pin.config().no);
		uint16_t tzSignal;
		switch (xbarInput)
		{
		case XBAR_INPUT1:
			tzSignal = EPWM_TZ_SIGNAL_OSHT1;
			break;
		case XBAR_INPUT2:
			tzSignal = EPWM_TZ_SIGNAL_OSHT2;
			break;

		case XBAR_INPUT3:
			tzSignal = EPWM_TZ_SIGNAL_OSHT3;
			break;
		default:
			tzSignal = EPWM_TZ_SIGNAL_OSHT3;
			break;
		}

		for (size_t i = 0; i < PhaseCount; ++i)
		{
			// Enable tzSignal as one shot trip source
			EPWM_enableTripZoneSignals(_module.base[i], tzSignal);
		}
	}
#endif

#ifdef CPU1
	/**
	 * @brief Transfers control over PWM unit to CPU2.
	 * @param (none)
	 * @return (none)
	 */
	void transferControlToCpu2()
	{
		SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);	// Disable sync(Freeze clock to PWM as well)

		for (size_t i = 0; i < PhaseCount; ++i)
		{
			GPIO_setMasterCore(_module.instance[i] * 2, GPIO_CORE_CPU2);
			GPIO_setMasterCore(_module.instance[i] * 2 + 1, GPIO_CORE_CPU2);
		}

		for (size_t i = 0; i < PhaseCount; ++i)
		{
			SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL0_EPWM,
					static_cast<uint16_t>(_module.instance[i])+1, SYSCTL_CPUSEL_CPU2);
		}

		SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);	// Enable sync and clock to PWM
	}
#endif

	/**
	 * @brief Returns base of PWM-unit.
	 * @param (none)
	 * @return Base of PWM-unit.
	 */
	uint32_t base() const { return _module.base[0]; }

	/**
	 * @brief Returns PWM time-base period value.
	 * @param (none)
	 * @return TBPRD register value.
	 */
	uint16_t period() const { return _period; }

	/**
	 * @brief Returns PWM frequency.
	 * @param (none)
	 * @return PWM frequency.
	 */
	float freq() const { return _switchingFreq; }

	/**
	 * @brief Sets PWM frequency.
	 * @param freq - PWM frequency
	 * @return (none)
	 */
	void setFreq(float freq)
	{
		_switchingFreq = freq;
		switch (_counterMode)
		{
		case PWM_COUNTER_MODE_UP:
		case PWM_COUNTER_MODE_DOWN:
			_period = (_timebaseClkFreq / _switchingFreq) - 1;
			break;
		case PWM_COUNTER_MODE_UP_DOWN:
			_period = (_timebaseClkFreq / _switchingFreq) / 2;
			break;
		}

		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setTimeBasePeriod(_module.base[i], _period);
		}
	}

	/**
	 * @brief Sets counter compare values.
	 * @param cmpValues - compare values array pointer
	 * @return (none)
	 */
	void setCompareValues(const uint16_t cmpValues[])
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            cmpValues[i]);
		}
	}

	/**
	 * @overload
	 */
	void setCompareValues(const emb::Array<uint16_t, Phases>& cmpValues)
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            cmpValues[i]);
		}
	}

	/**
	 * @brief Sets one counter compare value for all modules.
	 * @param cmpValue - compare value
	 * @return (none)
	 */
	void setCompareValue(uint16_t cmpValue)
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            cmpValue);
		}
	}

	/**
	 * @brief Updates counter compare values.
	 * @param dutyCycles - reference to duty cycles array
	 * @return (none)
	 */
	void setDutyCycles(const emb::Array<float, Phases>& dutyCycles)
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            static_cast<uint16_t>(dutyCycles[i] * _period));
		}
	}

	/**
	 * @brief Updates counter compare values.
	 * @param dutyCycle - duty cycle value for all modules
	 * @return (none)
	 */
	void setDutyCycle(float dutyCycle)
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            static_cast<uint16_t>(dutyCycle * _period));
		}
	}

	/**
	 * @brief Starts PWM.
	 * @param (none)
	 * @return (none)
	 */
	void start()
	{
		_state = State::On;
		for (size_t i = 0; i < Phases; ++i)
		{
			EPWM_clearTripZoneFlag(_module.base[i], EPWM_TZ_INTERRUPT | EPWM_TZ_FLAG_OST);
		}
	}

	/**
	 * @brief Stops PWM.
	 * @param (none)
	 * @return (none)
	 */
	void stop()
	{
		for (size_t i = 0; i < Phases; ++i)
		{
			EPWM_forceTripZoneEvent(_module.base[i], EPWM_TZ_FORCE_EVENT_OST);
		}
		_state = State::Off;
	}

	/**
	 * @brief Returns PWM state.
	 * @param (none)
	 * @return PWM state.
	 */
	State state() const
	{
		return _state;
	}

/*============================================================================*/
/*============================ Interrupts ====================================*/
/*============================================================================*/
	/**
	 * @brief Enables time-base interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableEventInterrupts()
	{
		EPWM_enableInterrupt(_module.base[0]);
	}

	/**
	 * @brief Enables trip interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableTripInterrupts()
	{
		EPWM_enableTripZoneInterrupt(_module.base[0], EPWM_TZ_INTERRUPT_OST);
	}

	/**
	 * @brief Disables time-base interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableEventInterrupts()
	{
		EPWM_disableInterrupt(_module.base[0]);
	}

	/**
	 * @brief Disables trip interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableTripInterrupts()
	{
		EPWM_disableTripZoneInterrupt(_module.base[0], EPWM_TZ_INTERRUPT_OST);
	}

	/**
	 * @param Registers time-base interrupt handler
	 * @param handler - pointer to handler
	 * @return (none)
	 */
	void registerEventInterruptHandler(void (*handler)(void))
	{
		Interrupt_register(_module.pieEventIntNum, handler);
		Interrupt_enable(_module.pieEventIntNum);
	}

	/**
	 * @param Registers trip event interrupt handler
	 * @param handler - pointer to handler
	 * @return (none)
	 */
	void registerTripInterruptHandler(void (*handler)(void))
	{
		Interrupt_register(_module.pieTripIntNum, handler);
		Interrupt_enable(_module.pieTripIntNum);
	}

	/**
	 * @brief Acknowledges time-base interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void acknowledgeEventInterrupt()
	{
		EPWM_clearEventTriggerInterruptFlag(_module.base[0]);
		Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
	}

	/**
	 * @brief Acknowledges trip event interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void acknowledgeTripInterrupt()
	{
		Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP2);
	}

protected:
#ifdef CPU1
	void _initPins(const pwm::Config<Phases>& config)
	{
		for (size_t i = 0; i < Phases; ++i)
		{
			GPIO_setPadConfig(config.peripheral[i].underlying_value() * 2, GPIO_PIN_TYPE_STD);
			GPIO_setPadConfig(config.peripheral[i].underlying_value() * 2 + 1, GPIO_PIN_TYPE_STD);
			GPIO_setPinConfig(impl::pwmPinOutAConfigs[config.peripheral[i].underlying_value()]);
			GPIO_setPinConfig(impl::pwmPinOutBConfigs[config.peripheral[i].underlying_value()]);
		}
	}
#endif

	/**
	 * @brief Initializes user-defined custom options.
	 *
	 */
	void _initCustomOptions()
	{
		// place custom options here
	}
};


/// @}
} // namespace pwm


} // namespace mcu


