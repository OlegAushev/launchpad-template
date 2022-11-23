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
void impl::initChannels(emb::Array<impl::Channel, ChannelName::AdcChannelCount>& channels)
{
	channels[ChannelName::AdcCurrentPhaseU] = impl::Channel(
			ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN0, ADC_SOC_NUMBER0, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::AdcCurrentPhaseV] = impl::Channel(
			ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN1, ADC_SOC_NUMBER1, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::AdcCurrentPhaseW] = impl::Channel(
			ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN2, ADC_SOC_NUMBER2, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::AdcCurrentPhaseX] = impl::Channel(
			ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN5, ADC_SOC_NUMBER3, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::AdcCurrentPhaseY] = impl::Channel(
			ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN4, ADC_SOC_NUMBER4, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::AdcCurrentPhaseZ] = impl::Channel(
			ADCB_BASE, ADCBRESULT_BASE, ADC_CH_ADCIN3, ADC_SOC_NUMBER5, ADC_TRIGGER_SW_ONLY);

	channels[ChannelName::AdcVoltageDC] = impl::Channel(
			ADCD_BASE, ADCDRESULT_BASE, ADC_CH_ADCIN0, ADC_SOC_NUMBER0, ADC_TRIGGER_SW_ONLY);

	channels[ChannelName::AdcTemperaturePhaseU] = impl::Channel(
			ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN14, ADC_SOC_NUMBER0, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::AdcTemperaturePhaseV] = impl::Channel(
			ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN15, ADC_SOC_NUMBER1, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::AdcTemperaturePhaseW] = impl::Channel(
			ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN2, ADC_SOC_NUMBER2, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::AdcTemperaturePhaseX] = impl::Channel(
			ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN3, ADC_SOC_NUMBER3, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::AdcTemperaturePhaseY] = impl::Channel(
			ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN4, ADC_SOC_NUMBER4, ADC_TRIGGER_SW_ONLY);
	channels[ChannelName::AdcTemperaturePhaseZ] = impl::Channel(
			ADCC_BASE, ADCCRESULT_BASE, ADC_CH_ADCIN5, ADC_SOC_NUMBER5, ADC_TRIGGER_SW_ONLY);

	channels[ChannelName::AdcTemperatureCase] = impl::Channel(
			ADCD_BASE, ADCDRESULT_BASE, ADC_CH_ADCIN1, ADC_SOC_NUMBER1, ADC_TRIGGER_SW_ONLY);
}


///
///
///
void impl::initIrqs(emb::Array<impl::Irq, IrqName::AdcIrqCount>& irqs)
{
	irqs[IrqName::AdcIrqCurrentUVW] = impl::Irq(
			ADCB_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER2, INT_ADCB2);
	irqs[IrqName::AdcIrqCurrentXYZ] = impl::Irq(
			ADCB_BASE, ADC_INT_NUMBER3, ADC_SOC_NUMBER5, INT_ADCB3);

	irqs[IrqName::AdcIrqVoltageDC] = impl::Irq(
			ADCD_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER0, INT_ADCD2);

	irqs[IrqName::AdcIrqTemperatureUVW] = impl::Irq(
			ADCC_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER2, INT_ADCC2);
	irqs[IrqName::AdcIrqTemperatureXYZ] = impl::Irq(
			ADCC_BASE, ADC_INT_NUMBER3, ADC_SOC_NUMBER5, INT_ADCC3);

	irqs[IrqName::AdcIrqTemperatureCase] = impl::Irq(
			ADCD_BASE, ADC_INT_NUMBER3, ADC_SOC_NUMBER1, INT_ADCD3);
}


/// @}
} // namespace adc


} // namespace mcu


