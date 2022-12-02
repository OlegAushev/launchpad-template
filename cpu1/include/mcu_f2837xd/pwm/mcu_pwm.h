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
struct Configuration
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
	static const uint32_t s_pwmClkFreq = DEVICE_SYSCLK_FREQ / 2;
	static const uint32_t s_pwmClkCycle_ns = 1000000000 / s_pwmClkFreq;
	const uint32_t m_timebaseClkFreq;
	const uint32_t m_timebaseCycle_ns;

	impl::Module<Phases> m_module;
	CounterMode m_counterMode;
	float m_switchingFreq;
	uint16_t m_deadtimeCycles;

	uint16_t m_period;		// TBPRD register value
	uint16_t m_phaseShift[Phases];	// TBPHS registers values

	State m_state;
public:
	/**
	 * @brief Initializes MCU PWM unit.
	 * @param conf - PWM config
	 * @param (none)
	 */
	Module(const pwm::Configuration<Phases>& conf)
		: m_timebaseClkFreq(s_pwmClkFreq / conf.clockPrescaler)
		, m_timebaseCycle_ns(s_pwmClkCycle_ns * conf.clockPrescaler)
		, m_counterMode(conf.counterMode)
		, m_switchingFreq(conf.switchingFreq)
		, m_deadtimeCycles(conf.deadtime_ns / m_timebaseCycle_ns)
		, m_state(State::Off)
	{
		for (size_t i = 0; i < Phases; ++i)
		{
			m_module.instance[i] = conf.peripheral[i];
			m_module.base[i] = impl::pwmBases[conf.peripheral[i].underlying_value()];
		}
		m_module.pieEventIntNum = impl::pwmPieEventIntNums[conf.peripheral[0].underlying_value()];
		m_module.pieTripIntNum = impl::pwmPieTripIntNums[conf.peripheral[0].underlying_value()];

		for (size_t i = 0; i < Phases; ++i)
		{
			m_phaseShift[i] = 0;
		}

#ifdef CPU1
		_initPins(conf);
#else
		EMB_UNUSED(impl::pwmPinOutAConfigs);
		EMB_UNUSED(impl::pwmPinOutBConfigs);
#endif

		// Disable sync, freeze clock to PWM
		SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

		/* ========================================================================== */
		// Calculate TBPRD value
		switch (conf.counterMode.native_value())
		{
		case CounterMode::Up:
		case CounterMode::Down:
			m_period = (m_timebaseClkFreq / m_switchingFreq) - 1;
			break;
		case CounterMode::UpDown:
			m_period = (m_timebaseClkFreq / m_switchingFreq) / 2;
			break;
		}

		for (size_t i = 0; i < Phases; ++i)
		{
			EPWM_setTimeBasePeriod(m_module.base[i], m_period);
			EPWM_setTimeBaseCounter(m_module.base[i], 0);

			/* ========================================================================== */
			// Clock prescaler
			EPWM_setClockPrescaler(m_module.base[i],
					static_cast<EPWM_ClockDivider>(conf.clkDivider.underlying_value()),
					static_cast<EPWM_HSClockDivider>(conf.hsclkDivider.underlying_value()));

			/* ========================================================================== */
			// Compare values
			EPWM_setCounterCompareValue(m_module.base[i], EPWM_COUNTER_COMPARE_A, 0);

			/* ========================================================================== */
			// Counter mode
			EPWM_setTimeBaseCounterMode(m_module.base[i],
					static_cast<EPWM_TimeBaseCountMode>(conf.counterMode.underlying_value()));

#ifdef CPU1
			/* ========================================================================== */
			// Sync input source for the EPWM signals
			switch (m_module.base[i])
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
				if ((i == 0) && (m_module.base[i] == EPWM1_BASE))
				{
					// EPWM1 is master
					EPWM_setSyncOutPulseMode(m_module.base[i], EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO);
				}
				else
				{
					// other modules sync is pass-through
					EPWM_setSyncOutPulseMode(m_module.base[i], EPWM_SYNC_OUT_PULSE_ON_EPWMxSYNCIN);
				}
				break;
			case PhaseCount::One:
				if (m_module.base[i] == EPWM1_BASE)
				{
					EPWM_setSyncOutPulseMode(m_module.base[i], EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO);
				}
				else
				{
					EPWM_setSyncOutPulseMode(m_module.base[i], EPWM_SYNC_OUT_PULSE_ON_EPWMxSYNCIN);
				}
				break;
			}

			/* ========================================================================== */
			// Time-base counter synchronization and phase shift
			switch (Phases)
			{
			case PhaseCount::Six:
			case PhaseCount::Three:
				if ((i == 0) && (m_module.base[i] == EPWM1_BASE))
				{
					// EPWM1 is master, EPWM4,7,10 are synced to it
					// master has no phase shift
					EPWM_disablePhaseShiftLoad(m_module.base[i]);
					EPWM_setPhaseShift(m_module.base[i], 0);
				}
				else
				{
					EPWM_enablePhaseShiftLoad(m_module.base[i]);
					// start counting up when sync occurs
					// used when the time-base counter is configured in the up-down-count mode
					EPWM_setCountModeAfterSync(m_module.base[i], EPWM_COUNT_MODE_UP_AFTER_SYNC);
					// 2 x EPWMCLK - delay from internal master module to slave modules, p.1876
					EPWM_setPhaseShift(m_module.base[i], 2 + m_phaseShift[i]);
				}
				break;

			case PhaseCount::One:
				if (m_module.base[i] == EPWM1_BASE)
				{
					EPWM_disablePhaseShiftLoad(m_module.base[i]);
					EPWM_setPhaseShift(m_module.base[i], 0);
				}
				else
				{
					EPWM_enablePhaseShiftLoad(m_module.base[i]);
					EPWM_setCountModeAfterSync(m_module.base[i], EPWM_COUNT_MODE_UP_AFTER_SYNC);
					EPWM_setPhaseShift(m_module.base[i], 2 + m_phaseShift[i]);
				}
				break;
			}

			/* ========================================================================== */
			// Shadowing
			EPWM_selectPeriodLoadEvent(m_module.base[i], EPWM_SHADOW_LOAD_MODE_COUNTER_ZERO);
			EPWM_setCounterCompareShadowLoadMode(m_module.base[i], EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);
			EPWM_setActionQualifierContSWForceShadowMode(m_module.base[i], EPWM_AQ_SW_IMMEDIATE_LOAD);

			/* ========================================================================== */
			// CMPA actions
				// PWMxA configuration for typical waveforms, change this if other is needed
			switch (conf.operatingMode.native_value())
			{
			case CounterMode::Up:
				EPWM_setActionQualifierAction(m_module.base[i],	EPWM_AQ_OUTPUT_A,
						EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
				EPWM_setActionQualifierAction(m_module.base[i], EPWM_AQ_OUTPUT_A,
						EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
				break;
			case CounterMode::Down:
				EPWM_setActionQualifierAction(m_module.base[i],	EPWM_AQ_OUTPUT_A,
						EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
				EPWM_setActionQualifierAction(m_module.base[i], EPWM_AQ_OUTPUT_A,
						EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);
				break;
			case CounterMode::UpDown:
				EPWM_setActionQualifierAction(m_module.base[i], EPWM_AQ_OUTPUT_A,
						EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
				EPWM_setActionQualifierAction(m_module.base[i], EPWM_AQ_OUTPUT_A,
						EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
				break;
			}
				// PWMxB configuration - always LOW: typically only PWMxA is used by dead-band submodule
			EPWM_setActionQualifierAction(m_module.base[i],	EPWM_AQ_OUTPUT_B,
					EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
			EPWM_setActionQualifierAction(m_module.base[i], EPWM_AQ_OUTPUT_B,
					EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
			EPWM_setActionQualifierAction(m_module.base[i], EPWM_AQ_OUTPUT_B,
					EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
			EPWM_setActionQualifierAction(m_module.base[i], EPWM_AQ_OUTPUT_B,
					EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);

			/* ========================================================================== */
			// Dead-Band
			EPWM_setDeadBandControlShadowLoadMode(m_module.base[i], EPWM_DB_LOAD_ON_CNTR_ZERO);
			EPWM_setDeadBandDelayMode(m_module.base[i], EPWM_DB_FED, true);
			EPWM_setDeadBandDelayMode(m_module.base[i], EPWM_DB_RED, true);

			switch (conf.operatingMode.native_value())
			{
			case OperatingMode::ActiveHighComplementary:
				EPWM_setDeadBandDelayPolarity(m_module.base[i], EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_HIGH);
				EPWM_setDeadBandDelayPolarity(m_module.base[i], EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW);
				break;
			case OperatingMode::ActiveLowComplementary:
				EPWM_setDeadBandDelayPolarity(m_module.base[i], EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_LOW);
				EPWM_setDeadBandDelayPolarity(m_module.base[i], EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_HIGH);
				break;
			}

			EPWM_setRisingEdgeDeadBandDelayInput(m_module.base[i], EPWM_DB_INPUT_EPWMA);
			EPWM_setFallingEdgeDeadBandDelayInput(m_module.base[i], EPWM_DB_INPUT_EPWMA);
			EPWM_setRisingEdgeDelayCount(m_module.base[i], m_deadtimeCycles);
			EPWM_setFallingEdgeDelayCount(m_module.base[i], m_deadtimeCycles);
			EPWM_setDeadBandCounterClock(m_module.base[i], EPWM_DB_COUNTER_CLOCK_FULL_CYCLE);

			switch (conf.outputSwap.native_value())
			{
			case OutputSwap::No:
				EPWM_setDeadBandOutputSwapMode(m_module.base[i], EPWM_DB_OUTPUT_A, false);
				EPWM_setDeadBandOutputSwapMode(m_module.base[i], EPWM_DB_OUTPUT_B, false);
				break;
			case OutputSwap::Yes:
				EPWM_setDeadBandOutputSwapMode(m_module.base[i], EPWM_DB_OUTPUT_A, true);
				EPWM_setDeadBandOutputSwapMode(m_module.base[i], EPWM_DB_OUTPUT_B, true);
				break;
			}

			/* ========================================================================== */
			// Trip-Zone actions
			switch (conf.operatingMode.native_value())
			{
			case OperatingMode::ActiveHighComplementary:
				EPWM_setTripZoneAction(m_module.base[i], EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
				EPWM_setTripZoneAction(m_module.base[i], EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_LOW);
				break;
			case OperatingMode::ActiveLowComplementary:
				EPWM_setTripZoneAction(m_module.base[i], EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_HIGH);
				EPWM_setTripZoneAction(m_module.base[i], EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_HIGH);
				break;
			}

			EPWM_clearOneShotTripZoneFlag(m_module.base[i], EPWM_TZ_OST_FLAG_OST1);
			EPWM_clearTripZoneFlag(m_module.base[i], EPWM_TZ_INTERRUPT | EPWM_TZ_FLAG_OST);
		}

		/* ========================================================================== */
		// ADC Trigger configuration, only first module triggers ADC
		if (conf.adcTriggerEnable[0])
		{
			EPWM_setADCTriggerSource(m_module.base[0], EPWM_SOC_A, conf.adcTriggerSource[0]);
			EPWM_setADCTriggerEventPrescale(m_module.base[0], EPWM_SOC_A, 1);
			EPWM_enableADCTrigger(m_module.base[0], EPWM_SOC_A);
		}

		if (conf.adcTriggerEnable[1])
		{
			EPWM_setADCTriggerSource(m_module.base[0], EPWM_SOC_B, conf.adcTriggerSource[1]);
			EPWM_setADCTriggerEventPrescale(m_module.base[0], EPWM_SOC_B, 1);
			EPWM_enableADCTrigger(m_module.base[0], EPWM_SOC_B);
		}

		/* ========================================================================== */
		// Interrupts, only interrupt on first module is required
		EPWM_setInterruptSource(m_module.base[0], conf.eventInterruptSource);
		EPWM_setInterruptEventCount(m_module.base[0], 1U);

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
			EPWM_enableTripZoneSignals(m_module.base[i], tzSignal);
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
			GPIO_setMasterCore(m_module.instance[i] * 2, GPIO_CORE_CPU2);
			GPIO_setMasterCore(m_module.instance[i] * 2 + 1, GPIO_CORE_CPU2);
		}

		for (size_t i = 0; i < PhaseCount; ++i)
		{
			SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL0_EPWM,
					static_cast<uint16_t>(m_module.instance[i])+1, SYSCTL_CPUSEL_CPU2);
		}

		SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);	// Enable sync and clock to PWM
	}
#endif

	/**
	 * @brief Returns base of PWM-unit.
	 * @param (none)
	 * @return Base of PWM-unit.
	 */
	uint32_t base() const { return m_module.base[0]; }

	/**
	 * @brief Returns PWM time-base period value.
	 * @param (none)
	 * @return TBPRD register value.
	 */
	uint16_t period() const { return m_period; }

	/**
	 * @brief Returns PWM frequency.
	 * @param (none)
	 * @return PWM frequency.
	 */
	float freq() const { return m_switchingFreq; }

	/**
	 * @brief Sets PWM frequency.
	 * @param freq - PWM frequency
	 * @return (none)
	 */
	void setFreq(float freq)
	{
		m_switchingFreq = freq;
		switch (m_counterMode)
		{
		case PWM_COUNTER_MODE_UP:
		case PWM_COUNTER_MODE_DOWN:
			m_period = (m_timebaseClkFreq / m_switchingFreq) - 1;
			break;
		case PWM_COUNTER_MODE_UP_DOWN:
			m_period = (m_timebaseClkFreq / m_switchingFreq) / 2;
			break;
		}

		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setTimeBasePeriod(m_module.base[i], m_period);
		}
	}

	/**
	 * @brief Sets counter compare values.
	 * @param cmpValues - compare values array pointer
	 * @return (none)
	 */
	void setCompareValues(const uint16_t cmpValues[]) const
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(m_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            cmpValues[i]);
		}
	}

	/**
	 * @overload
	 */
	void setCompareValues(const emb::Array<uint16_t, Phases>& cmpValues) const
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(m_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            cmpValues[i]);
		}
	}

	/**
	 * @brief Sets one counter compare value for all modules.
	 * @param cmpValue - compare value
	 * @return (none)
	 */
	void setCompareValue(uint16_t cmpValue) const
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(m_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            cmpValue);
		}
	}

	/**
	 * @brief Updates counter compare values.
	 * @param dutyCycles - reference to duty cycles array
	 * @return (none)
	 */
	void setDutyCycles(const emb::Array<float, Phases>& dutyCycles) const
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(m_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            static_cast<uint16_t>(dutyCycles[i] * m_period));
		}
	}

	/**
	 * @brief Updates counter compare values.
	 * @param dutyCycle - duty cycle value for all modules
	 * @return (none)
	 */
	void setDutyCycle(float dutyCycle) const
	{
		for (size_t i = 0; i < PhaseCount; ++i)
		{
			EPWM_setCounterCompareValue(m_module.base[i],
			                            EPWM_COUNTER_COMPARE_A,
			                            static_cast<uint16_t>(dutyCycle * m_period));
		}
	}

	/**
	 * @brief Starts PWM.
	 * @param (none)
	 * @return (none)
	 */
	void start()
	{
		m_state = State::On;
		for (size_t i = 0; i < Phases; ++i)
		{
			EPWM_clearTripZoneFlag(m_module.base[i], EPWM_TZ_INTERRUPT | EPWM_TZ_FLAG_OST);
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
			EPWM_forceTripZoneEvent(m_module.base[i], EPWM_TZ_FORCE_EVENT_OST);
		}
		m_state = State::Off;
	}

	/**
	 * @brief Returns PWM state.
	 * @param (none)
	 * @return PWM state.
	 */
	State state() const
	{
		return m_state;
	}

/*============================================================================*/
/*============================ Interrupts ====================================*/
/*============================================================================*/
	/**
	 * @brief Enables time-base interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableEventInterrupts() const
	{
		EPWM_enableInterrupt(m_module.base[0]);
	}

	/**
	 * @brief Enables trip interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableTripInterrupts() const
	{
		EPWM_enableTripZoneInterrupt(m_module.base[0], EPWM_TZ_INTERRUPT_OST);
	}

	/**
	 * @brief Disables time-base interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableEventInterrupts() const
	{
		EPWM_disableInterrupt(m_module.base[0]);
	}

	/**
	 * @brief Disables trip interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableTripInterrupts() const
	{
		EPWM_disableTripZoneInterrupt(m_module.base[0], EPWM_TZ_INTERRUPT_OST);
	}

	/**
	 * @param Registers time-base interrupt handler
	 * @param handler - pointer to handler
	 * @return (none)
	 */
	void registerEventInterruptHandler(void (*handler)(void)) const
	{
		Interrupt_register(m_module.pieEventIntNum, handler);
		Interrupt_enable(m_module.pieEventIntNum);
	}

	/**
	 * @param Registers trip event interrupt handler
	 * @param handler - pointer to handler
	 * @return (none)
	 */
	void registerTripInterruptHandler(void (*handler)(void)) const
	{
		Interrupt_register(m_module.pieTripIntNum, handler);
		Interrupt_enable(m_module.pieTripIntNum);
	}

	/**
	 * @brief Acknowledges time-base interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void acknowledgeEventInterrupt() const
	{
		EPWM_clearEventTriggerInterruptFlag(m_module.base[0]);
		Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
	}

	/**
	 * @brief Acknowledges trip event interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void acknowledgeTripInterrupt() const
	{
		Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP2);
	}

protected:
#ifdef CPU1
	void _initPins(const pwm::Configuration<Phases>& conf)
	{
		for (size_t i = 0; i < Phases; ++i)
		{
			GPIO_setPadConfig(conf.peripheral[i].underlying_value() * 2, GPIO_PIN_TYPE_STD);
			GPIO_setPadConfig(conf.peripheral[i].underlying_value() * 2 + 1, GPIO_PIN_TYPE_STD);
			GPIO_setPinConfig(impl::pwmPinOutAConfigs[conf.peripheral[i].underlying_value()]);
			GPIO_setPinConfig(impl::pwmPinOutBConfigs[conf.peripheral[i].underlying_value()]);
		}
	}
#endif
};


/// @}
} // namespace pwm


} // namespace mcu


