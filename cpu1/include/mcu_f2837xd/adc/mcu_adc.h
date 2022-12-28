/**
 * @defgroup mcu_adc ADC
 * @ingroup mcu
 * 
 * @file mcu_adc.h
 * @ingroup mcu mcu_adc
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
#include "../adc/channels/mcu_adcchannels.h"
#include "emb/emb_core.h"
#include "emb/emb_array.h"


namespace mcu {


namespace adc {
/// @addtogroup mcu_adc
/// @{


/// ADC modules
SCOPED_ENUM_DECLARE_BEGIN(Peripheral)
{
	AdcA,
	AdcB,
	AdcC,
	AdcD
}
SCOPED_ENUM_DECLARE_END(Peripheral)


/**
 * @brief ADC unit config.
 */
struct Config
{
	uint32_t sampleWindow_ns;
};


namespace impl {


/**
 * @brief ADC module implementation.
 */
struct Module
{
	uint32_t base;
};


extern const uint32_t adcBases[4];
extern const uint16_t adcPieIntGroups[4];


/**
 * @brief ADC channel implementation.
 */
struct Channel
{
	uint32_t base;
	uint32_t resultBase;
	ADC_Channel channel;
	ADC_SOCNumber soc;
	ADC_Trigger trigger;
	Channel() {}
	Channel(uint32_t base_, uint32_t resultBase_, ADC_Channel channel_, ADC_SOCNumber soc_, ADC_Trigger trigger_)
		: base(base_)
		, resultBase(resultBase_)
		, channel(channel_)
		, soc(soc_)
		, trigger(trigger_)
	{}
};


/**
 * @brief ADC IRQ implementation.
 */
struct Irq
{
	uint32_t base;
	ADC_IntNumber intNum;
	ADC_SOCNumber soc;
	uint32_t pieIntNum;
	Irq() {}
	Irq(uint32_t base_, ADC_IntNumber intNum_, ADC_SOCNumber soc_, uint32_t pieIntNum_)
		: base(base_)
		, intNum(intNum_)
		, soc(soc_)
		, pieIntNum(pieIntNum_)
	{}
};


/**
 * @brief Initializes ADC channels.
 * @param channels - ADC channel array
 * @return (none)
 */
void initChannels(emb::Array<impl::Channel, ChannelName::Count>& channels);


/**
 * @brief Initializes ADC IRQs.
 * @param irqs - ADC IRQ array
 * @return (none)
 */
void initIrqs(emb::Array<impl::Irq, IrqName::Count>& irqs);


} // namespace impl


/**
 * @brief ADC unit class.
 */
class Module : public emb::c28x::interrupt_invoker<Module>, private emb::noncopyable
{
private:
	impl::Module _module[4];
	static emb::Array<impl::Channel, ChannelName::Count> _channels;
	static emb::Array<impl::Irq, IrqName::Count> _irqs;
	const uint32_t _sampleWindowCycles;
public:
	/**
	 * @brief Initializes MCU ADC module.
	 * @param conf - ADC config
	 */
	Module(const adc::Config& conf);

	/**
	 * @brief Starts conversion on specified channel.
	 * @param channel - ADC channel
	 * @return (none)
	 */
	void start(ChannelName channel)
	{
		ADC_forceSOC(_channels[channel.underlying_value()].base, _channels[channel.underlying_value()].soc);
	}

	/**
	 * @brief Returns result of ADC on specified channel.
	 * @param channel - ADC channel
	 * @return ADC-result raw data.
	 */
	uint16_t read(ChannelName channel) const
	{
		return ADC_readResult(_channels[channel.underlying_value()].resultBase, _channels[channel.underlying_value()].soc);
	}

/*============================================================================*/
/*============================ Interrupts ====================================*/
/*============================================================================*/
	/**
	 * @brief Enables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableInterrupts()
	{
		for (size_t i = 0; i < IrqName::Count; ++i)
		{
			Interrupt_enable(_irqs[i].pieIntNum);
		}
	}

	/**
	 * @brief Disables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableInterrupts()
	{
		for (size_t i = 0; i < IrqName::Count; ++i)
		{
			Interrupt_disable(_irqs[i].pieIntNum);
		}
	}

	/**
	 * @brief Registers ADC ISR
	 * @param irq - interrupt request
	 * @param handler - pointer to interrupt handler
	 * @return (none)
	 */
	void registerInterruptHandler(IrqName irq, void (*handler)(void))
	{
		Interrupt_register(_irqs[irq.underlying_value()].pieIntNum, handler);
	}

	/**
	 * @brief Acknowledges interrupt.
	 * @param irq - interrupt request
	 * @return (none)
	 */
	void acknowledgeInterrupt(IrqName irq)
	{
		ADC_clearInterruptStatus(_irqs[irq.underlying_value()].base, _irqs[irq.underlying_value()].intNum);
		Interrupt_clearACKGroup(impl::adcPieIntGroups[_irqs[irq.underlying_value()].intNum]);
	}

	/**
	 * @brief Returns interrupt status of IRQ.
	 * @param irq - interrupt request
	 * @return \c true if the interrupt flag is set and \c false if it is not.
	 */
	bool interruptPending(IrqName irq) const
	{
		return ADC_getInterruptStatus(_irqs[irq.underlying_value()].base, _irqs[irq.underlying_value()].intNum);
	}

	/**
	 * @brief Clears interrupt status of specified IRQ.
	 * @param (none)
	 * @return (none)
	 */
	void clearInterruptStatus(IrqName irq)
	{
		ADC_clearInterruptStatus(_irqs[irq.underlying_value()].base, _irqs[irq.underlying_value()].intNum);
	}
};


/**
 * @brief ADC channel class
 */
class Channel
{
public:
	Module* adc;
private:
	ChannelName _channelName;
public:
	/**
	 * @brief Initializes ADC channel.
	 * @param (none)
	 */
	Channel()
		: adc(Module::instance())
		, _channelName(ChannelName::Count)	// dummy write
	{}


	/**
	 * @brief Initializes ADC channel.
	 * @param channelName - channel name
	 */
	Channel(ChannelName channelName)
		: adc(Module::instance())
		, _channelName(channelName)
	{}


	/**
	 * @brief Initializes ADC channel.
	 * @param channelName - channel name
	 * @return (none)
	 */
	void init(ChannelName channelName)
	{
		_channelName = channelName;
	}


	/**
	 * @brief Starts conversion.
	 * @param (none)
	 * @return (none)
	 */
	void start()
	{
		adc->start(_channelName);
	}


	/**
	 * @brief Returns result of ADC.
	 * @param (none)
	 * @return ADC-result raw data.
	 */
	uint16_t read() const
	{
		return adc->read(_channelName);
	}
};


/// @}
} // namespace adc


} // namespace mcu


