/**
 * @file
 * @ingroup mcu mcu_adc
 */


#include "mcu_f2837xd/adc/mcu_adc.h"


namespace mcu {


namespace adc {


const uint32_t impl::adcBases[4] = {ADCA_BASE, ADCB_BASE, ADCC_BASE, ADCD_BASE};
const uint16_t impl::adcPieIntGroups[4] = {INTERRUPT_ACK_GROUP1, INTERRUPT_ACK_GROUP10,
		INTERRUPT_ACK_GROUP10, INTERRUPT_ACK_GROUP10};


emb::Array<impl::Channel, ChannelName::AdcChannelCount> Module::s_channels;
emb::Array<impl::Irq, IrqName::AdcIrqCount> Module::s_irqs;


///
///
///
Module::Module(const Config& cfg)
	: emb::c28x::singleton<Module>(this)
	, m_sampleWindowCycles(cfg.sampleWindow_ns / (1000000000 / mcu::sysclkFreq()))
{
	for (size_t i = 0; i < 4; ++i)
	{
		m_module[i].base = impl::adcBases[i];
	}

	impl::initChannels(s_channels);
	impl::initIrqs(s_irqs);

	for (uint32_t i = 0; i < 4; ++i)
	{
		ADC_setPrescaler(m_module[i].base, ADC_CLK_DIV_4_0);		// fclk(adc)max = 50 MHz
		ADC_setMode(m_module[i].base, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
		ADC_setInterruptPulseMode(m_module[i].base, ADC_PULSE_END_OF_CONV);
		ADC_enableConverter(m_module[i].base);
		ADC_setSOCPriority(m_module[i].base, ADC_PRI_ALL_HIPRI);	// SOCs at high priority - easier to control order
		mcu::delay_us(1000);						// delay for power-up
	}

	// Configure SOCs
	// For 12-bit resolution, a sampling window of (5 x SAMPLE_WINDOW_CYCLES)ns
	// at a 200MHz SYSCLK rate will be used
	for (size_t i = 0; i < s_channels.size(); ++i)
	{
		ADC_setupSOC(s_channels[i].base, s_channels[i].soc, s_channels[i].trigger,
				s_channels[i].channel, m_sampleWindowCycles);
	}

	// Interrupt config
	for (size_t i = 0; i < s_irqs.size(); ++i)
	{
		ADC_setInterruptSource(s_irqs[i].base, s_irqs[i].intNum, s_irqs[i].soc);
		ADC_enableInterrupt(s_irqs[i].base, s_irqs[i].intNum);
		ADC_clearInterruptStatus(s_irqs[i].base, s_irqs[i].intNum);
	}
}


} // namespace adc


} // namespace mcu


