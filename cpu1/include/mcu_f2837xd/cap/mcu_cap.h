/**
 * @defgroup mcu_cap CAP
 * @ingroup mcu
 * 
 * @file mcu_cap.h
 * @ingroup mcu mcu_cap
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


namespace mcu {
/// @addtogroup mcu_cap
/// @{


/// CAP modules
enum CapModule
{
	CAP1,
	CAP2,
	CAP3,
	CAP4,
	CAP5,
	CAP6
};


/**
 * @brief CAP config.
 */
template <unsigned int ChannelCount>
struct CapConfig
{
	CapModule module[ChannelCount];
	mcu::gpio::Input inputPin[ChannelCount];
};


namespace impl {


/**
 * @brief CAP module implementation.
 */
template <unsigned int ChannelCount>
struct CapModuleImpl
{
	CapModule instance[ChannelCount];
	uint32_t base[ChannelCount];
	XBAR_InputNum xbarInput[ChannelCount];
	uint32_t pieIntNum[ChannelCount];
};


extern const uint32_t capBases[6];
extern const XBAR_InputNum capXbarInputs[6];
extern const uint32_t capPieIntNums[6];


} // namespace impl


/**
 * @brief CAP unit class.
 */
template <unsigned int ChannelCount>
class Cap
{
private:
	impl::CapModuleImpl<ChannelCount> m_module;

private:
	Cap(const Cap& other);			// no copy constructor
	Cap& operator=(const Cap& other);	// no copy assignment operator
public:
	/**
	 * @brief Initializes MCU CAP unit.
	 * @param conf - CAP unit config
	 */
	Cap(const CapConfig<ChannelCount>& conf)
	{
		EMB_STATIC_ASSERT(ChannelCount > 0);
		EMB_STATIC_ASSERT(ChannelCount <= 6);

		// XBAR setup
		for (size_t i = 0; i < ChannelCount; ++i)
		{
			m_module.instance[i] = conf.module[i];
			m_module.base[i] = impl::capBases[conf.module[i]];
			m_module.xbarInput[i] = impl::capXbarInputs[conf.module[i]];
			m_module.pieIntNum[i] = impl::capPieIntNums[conf.module[i]];
			XBAR_setInputPin(m_module.xbarInput[i], conf.inputPin[i].config().no);
		}

		/* ECAP setup */
		for (size_t i = 0; i < ChannelCount; ++i)
		{
			ECAP_disableInterrupt(m_module.base[i],
								  (ECAP_ISR_SOURCE_CAPTURE_EVENT_1  |
								   ECAP_ISR_SOURCE_CAPTURE_EVENT_2  |
								   ECAP_ISR_SOURCE_CAPTURE_EVENT_3  |
								   ECAP_ISR_SOURCE_CAPTURE_EVENT_4  |
								   ECAP_ISR_SOURCE_COUNTER_OVERFLOW |
								   ECAP_ISR_SOURCE_COUNTER_PERIOD   |
								   ECAP_ISR_SOURCE_COUNTER_COMPARE));
			ECAP_clearInterrupt(m_module.base[i],
								(ECAP_ISR_SOURCE_CAPTURE_EVENT_1  |
								 ECAP_ISR_SOURCE_CAPTURE_EVENT_2  |
								 ECAP_ISR_SOURCE_CAPTURE_EVENT_3  |
								 ECAP_ISR_SOURCE_CAPTURE_EVENT_4  |
								 ECAP_ISR_SOURCE_COUNTER_OVERFLOW |
								 ECAP_ISR_SOURCE_COUNTER_PERIOD   |
								 ECAP_ISR_SOURCE_COUNTER_COMPARE));

			ECAP_disableTimeStampCapture(m_module.base[i]);

			ECAP_stopCounter(m_module.base[i]);
			ECAP_enableCaptureMode(m_module.base[i]);
			ECAP_setCaptureMode(m_module.base[i], ECAP_CONTINUOUS_CAPTURE_MODE, ECAP_EVENT_2);


			ECAP_setEventPolarity(m_module.base[i], ECAP_EVENT_1, ECAP_EVNT_RISING_EDGE);
			ECAP_setEventPolarity(m_module.base[i], ECAP_EVENT_2, ECAP_EVNT_FALLING_EDGE);

			ECAP_enableCounterResetOnEvent(m_module.base[i], ECAP_EVENT_1);
			ECAP_enableCounterResetOnEvent(m_module.base[i], ECAP_EVENT_2);

			ECAP_setSyncOutMode(m_module.base[i], ECAP_SYNC_OUT_DISABLED);
			ECAP_startCounter(m_module.base[i]);
			ECAP_enableTimeStampCapture(m_module.base[i]);
			ECAP_reArm(m_module.base[i]);
		}
	}

	/**
	 * @brief Rearms unit.
	 * @param (none)
	 * @return (none)
	 */
	void rearm() const
	{
		for (int i = 0; i < ChannelCount; ++i)
		{
			ECAP_reArm(m_module.base[i]);
		}
	}

	/**
	 * @brief Enables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableInterrupts() const
	{
		for (int i = 0; i < ChannelCount; ++i)
		{
			ECAP_enableInterrupt(m_module.base[i], ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
			ECAP_enableInterrupt(m_module.base[i], ECAP_ISR_SOURCE_CAPTURE_EVENT_2);
			ECAP_enableInterrupt(m_module.base[i], ECAP_ISR_SOURCE_COUNTER_OVERFLOW);
		}
	}

	/**
	 * @brief Disables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableInterrupts() const
	{
		for (int i = 0; i < ChannelCount; ++i)
		{
			ECAP_disableInterrupt(m_module.base[i], ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
			ECAP_disableInterrupt(m_module.base[i], ECAP_ISR_SOURCE_CAPTURE_EVENT_2);
			ECAP_disableInterrupt(m_module.base[i], ECAP_ISR_SOURCE_COUNTER_OVERFLOW);
		}
	}

	/**
	 * @brief Registers interrupt handler.
	 * @param iCap - capture module
	 * @param handler - interrupt handler
	 * @return (none)
	 */
	void registerInterruptHandler(CapModule module, void (*handler)(void)) const
	{
		for (int i = 0; i < ChannelCount; ++i)
		{
			if (m_module.instance[i] == module)
			{
				Interrupt_register(m_module.pieIntNum[i], handler);
				Interrupt_enable(m_module.pieIntNum[i]);
				return;
			}
		}
	}
};


/// @}
} // namespace mcu


