/**
 * @file
 * @ingroup mcu mcu_adc
 */


#include <mcu_f2837xd/adc/mcu_adc.h>


namespace mcu {


namespace adc {
/// @addtogroup mcu_adc
/// @{


///
///
///
void impl::initChannels(emb::Array<impl::Channel, ChannelName::Count>& channels)
{
	channels[ChannelName::CurrentPhaseU] = impl::Channel(
			ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN0, ADC_SOC_NUMBER0, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::CurrentPhaseV] = impl::Channel(
			ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN1, ADC_SOC_NUMBER1, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::CurrentPhaseW] = impl::Channel(
			ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN2, ADC_SOC_NUMBER2, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::CurrentPhaseX] = impl::Channel(
			ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN5, ADC_SOC_NUMBER3, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::CurrentPhaseY] = impl::Channel(
			ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN4, ADC_SOC_NUMBER4, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::CurrentPhaseZ] = impl::Channel(
			ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN3, ADC_SOC_NUMBER5, ADC_TRIGGER_SW_ONLY);

	channels[ChannelName::VoltageDC] = impl::Channel(
			ADCD_BASE, ADCDRESULT_BASE, ADC_CH_ADCIN0, ADC_SOC_NUMBER0, ADC_TRIGGER_SW_ONLY);

	channels[ChannelName::TemperaturePhaseU] = impl::Channel(
			ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN14, ADC_SOC_NUMBER0, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::TemperaturePhaseV] = impl::Channel(
			ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN15, ADC_SOC_NUMBER1, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::TemperaturePhaseW] = impl::Channel(
			ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN2, ADC_SOC_NUMBER2, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::TemperaturePhaseX] = impl::Channel(
			ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN3, ADC_SOC_NUMBER3, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::TemperaturePhaseY] = impl::Channel(
			ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN4, ADC_SOC_NUMBER4, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::TemperaturePhaseZ] = impl::Channel(
			ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN5, ADC_SOC_NUMBER5, ADC_TRIGGER_SW_ONLY);

	channels[ChannelName::TemperatureCase] = impl::Channel(
			ADCD_BASE, ADCDRESULT_BASE, ADC_CH_ADCIN1, ADC_SOC_NUMBER1, ADC_TRIGGER_SW_ONLY);
}


///
///
///
void impl::initIrqs(emb::Array<impl::Irq, IrqName::Count>& irqs)
{
	irqs[IrqName::CurrentUVW] = impl::Irq(
			ADCB_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER2, INT_ADCB2);
	irqs[IrqName::CurrentXYZ] = impl::Irq(
			ADCB_BASE, ADC_INT_NUMBER3, ADC_SOC_NUMBER5, INT_ADCB3);

	irqs[IrqName::VoltageDC] = impl::Irq(
			ADCD_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER0, INT_ADCD2);

	irqs[IrqName::TemperatureUVW] = impl::Irq(
			ADCC_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER2, INT_ADCC2);
	irqs[IrqName::TemperatureXYZ] = impl::Irq(
			ADCC_BASE, ADC_INT_NUMBER3, ADC_SOC_NUMBER5, INT_ADCC3);

	irqs[IrqName::TemperatureCase] = impl::Irq(
			ADCD_BASE, ADC_INT_NUMBER3, ADC_SOC_NUMBER1, INT_ADCD3);
}


/// @}
} // namespace adc


} // namespace mcu


