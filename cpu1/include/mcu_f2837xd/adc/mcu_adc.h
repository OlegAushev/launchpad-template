/**
 * @defgroup mcu_adc ADC
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_adc
 */


#pragma once


#include "driverlib.h"
#include "device.h"
#include "emb/emb_core.h"
#include "emb/emb_array.h"
#include "../system/mcu_system.h"
#include "../adc/channels/mcu_adcchannels.h"


namespace mcu {


namespace adc {
/// @addtogroup mcu_adc
/// @{


/// ADC modules
enum Peripheral
{
	ADCA,
	ADCB,
	ADCC,
	ADCD
};


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
	Channel(uint32_t _base, uint32_t _resultBase, ADC_Channel _channel, ADC_SOCNumber _soc, ADC_Trigger _trigger)
		: base(_base)
		, resultBase(_resultBase)
		, channel(_channel)
		, soc(_soc)
		, trigger(_trigger)
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
	Irq(uint32_t _base, ADC_IntNumber _intNum, ADC_SOCNumber _soc, uint32_t _pieIntNum)
		: base(_base)
		, intNum(_intNum)
		, soc(_soc)
		, pieIntNum(_pieIntNum)
	{}
};


/**
 * @brief Initializes ADC channels.
 * @param channels - ADC channel array
 * @return (none)
 */
void initChannels(emb::Array<impl::Channel, ADC_CHANNEL_COUNT>& channels);


/**
 * @brief Initializes ADC IRQs.
 * @param irqs - ADC IRQ array
 * @return (none)
 */
void initIrqs(emb::Array<impl::Irq, ADC_IRQ_COUNT>& irqs);


} // namespace impl


/**
 * @brief ADC unit class.
 */
class Adc : public emb::c28x::singleton<Adc>
{
private:
	impl::Module m_module[4];

	static emb::Array<impl::Channel, ADC_CHANNEL_COUNT> s_channels;
	static emb::Array<impl::Irq, ADC_IRQ_COUNT> s_irqs;

	const uint32_t SAMPLE_WINDOW_CYCLES;

private:
	Adc(const Adc& other);			// no copy constructor
	Adc& operator=(const Adc& other);	// no copy assignment operator
public:
	/**
	 * @brief Initializes MCU ADC unit.
	 * @param cfg - ADC config
	 */
	Adc(const Config& cfg);

	/**
	 * @brief Starts conversion on specified channel.
	 * @param channel - ADC channel
	 * @return (none)
	 */
	void start(ChannelName channel) const
	{
		ADC_forceSOC(s_channels[channel].base, s_channels[channel].soc);
	}

	/**
	 * @brief Returns result of ADC on specified channel.
	 * @param channel - ADC channel
	 * @return ADC-result raw data.
	 */
	uint16_t read(ChannelName channel) const
	{
		return ADC_readResult(s_channels[channel].resultBase, s_channels[channel].soc);
	}

/*============================================================================*/
/*============================ Interrupts ====================================*/
/*============================================================================*/
	/**
	 * @brief Enables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableInterrupts() const
	{
		for (size_t i = 0; i < ADC_IRQ_COUNT; ++i)
		{
			Interrupt_enable(s_irqs[i].pieIntNum);
		}
	}

	/**
	 * @brief Disables interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableInterrupts() const
	{
		for (size_t i = 0; i < ADC_IRQ_COUNT; ++i)
		{
			Interrupt_disable(s_irqs[i].pieIntNum);
		}
	}

	/**
	 * @brief Registers ADC ISR
	 * @param irq - interrupt request
	 * @param handler - pointer to interrupt handler
	 * @return (none)
	 */
	void registerInterruptHandler(IrqName irq, void (*handler)(void)) const
	{
		Interrupt_register(s_irqs[irq].pieIntNum, handler);
	}

	/**
	 * @brief Acknowledges interrupt.
	 * @param irq - interrupt request
	 * @return (none)
	 */
	void acknowledgeInterrupt(IrqName irq) const
	{
		ADC_clearInterruptStatus(s_irqs[irq].base, s_irqs[irq].intNum);
		Interrupt_clearACKGroup(impl::adcPieIntGroups[s_irqs[irq].intNum]);
	}

	/**
	 * @brief Returns interrupt status of IRQ.
	 * @param irq - interrupt request
	 * @return \c true if the interrupt flag is set and \c false if it is not.
	 */
	bool interruptPending(IrqName irq)
	{
		return ADC_getInterruptStatus(s_irqs[irq].base, s_irqs[irq].intNum);
	}

	/**
	 * @brief Clears interrupt status of specified IRQ.
	 * @param (none)
	 * @return (none)
	 */
	void clearInterruptStatus(IrqName irq)
	{
		ADC_clearInterruptStatus(s_irqs[irq].base, s_irqs[irq].intNum);
	}
};


/**
 * @brief ADC channel class
 */
class Channel
{
public:
	Adc* adc;
private:
	ChannelName m_channelName;
public:
	/**
	 * @brief Initializes ADC channel.
	 * @param (none)
	 */
	Channel()
		: adc(Adc::instance())
		, m_channelName(ADC_CHANNEL_COUNT)	// dummy write
	{}


	/**
	 * @brief Initializes ADC channel.
	 * @param channelName - channel name
	 */
	Channel(ChannelName channelName)
		: adc(Adc::instance())
		, m_channelName(channelName)
	{}


	/**
	 * @brief Initializes ADC channel.
	 * @param channelName - channel name
	 * @return (none)
	 */
	void init(ChannelName channelName)
	{
		m_channelName = channelName;
	}


	/**
	 * @brief Starts conversion.
	 * @param (none)
	 * @return (none)
	 */
	void start() const
	{
		adc->start(m_channelName);
	}


	/**
	 * @brief Returns result of ADC.
	 * @param (none)
	 * @return ADC-result raw data.
	 */
	uint16_t read() const
	{
		return adc->read(m_channelName);
	}
};


/// @}
} // namespace adc


} // namespace mcu


