/**
 * @file mcu_adc.cpp
 * @ingroup mcu mcu_adc
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "mcu_f2837xd/adc/mcu_adc.h"


namespace mcu {


namespace adc {


const uint32_t impl::adcBases[4] = {ADCA_BASE, ADCB_BASE, ADCC_BASE, ADCD_BASE};
const uint16_t impl::adcPieIntGroups[4] = {INTERRUPT_ACK_GROUP1, INTERRUPT_ACK_GROUP10,
		INTERRUPT_ACK_GROUP10, INTERRUPT_ACK_GROUP10};


emb::Array<impl::Channel, ChannelName::Count> Module::_channels;
emb::Array<impl::Irq, IrqName::Count> Module::_irqs;


///
///
///
Module::Module(const adc::Config& conf)
	: emb::c28x::interrupt_invoker<Module>(this)
	, _sampleWindowCycles(conf.sampleWindow_ns / (1000000000 / mcu::sysclkFreq()))
{
	for (size_t i = 0; i < 4; ++i)
	{
		_module[i].base = impl::adcBases[i];
	}

	impl::initChannels(_channels);
	impl::initIrqs(_irqs);

	for (uint32_t i = 0; i < 4; ++i)
	{
		ADC_setPrescaler(_module[i].base, ADC_CLK_DIV_4_0);		// fclk(adc)max = 50 MHz
		ADC_setMode(_module[i].base, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
		ADC_setInterruptPulseMode(_module[i].base, ADC_PULSE_END_OF_CONV);
		ADC_enableConverter(_module[i].base);
		ADC_setSOCPriority(_module[i].base, ADC_PRI_ALL_HIPRI);	// SOCs at high priority - easier to control order
		mcu::delay_us(1000);						// delay for power-up
	}

	// Configure SOCs
	// For 12-bit resolution, a sampling window of (5 x SAMPLE_WINDOW_CYCLES)ns
	// at a 200MHz SYSCLK rate will be used
	for (size_t i = 0; i < _channels.size(); ++i)
	{
		ADC_setupSOC(_channels[i].base, _channels[i].soc, _channels[i].trigger,
				_channels[i].channel, _sampleWindowCycles);
	}

	// Interrupt config
	for (size_t i = 0; i < _irqs.size(); ++i)
	{
		ADC_setInterruptSource(_irqs[i].base, _irqs[i].intNum, _irqs[i].soc);
		ADC_enableInterrupt(_irqs[i].base, _irqs[i].intNum);
		ADC_clearInterruptStatus(_irqs[i].base, _irqs[i].intNum);
	}
}


} // namespace adc


} // namespace mcu


