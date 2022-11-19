/**
 * @file
 * @ingroup mcu mcu_adc
 */


#pragma once


namespace mcu {


namespace adc {
/// @addtogroup mcu_adc
/// @{


/// ADC channels (application-specific)
enum ChannelName
{
	ADC_CURRENT_PHASE_U,
	ADC_CURRENT_PHASE_V,
	ADC_CURRENT_PHASE_W,
	ADC_CURRENT_PHASE_X,
	ADC_CURRENT_PHASE_Y,
	ADC_CURRENT_PHASE_Z,

	ADC_VOLTAGE_DC,

	ADC_TEMPERATURE_PHASE_U,
	ADC_TEMPERATURE_PHASE_V,
	ADC_TEMPERATURE_PHASE_W,
	ADC_TEMPERATURE_PHASE_X,
	ADC_TEMPERATURE_PHASE_Y,
	ADC_TEMPERATURE_PHASE_Z,
	ADC_TEMPERATURE_CASE,

	ADC_CHANNEL_COUNT
};


/// ADC interrupt request source (application-specific).
enum IrqName
{
	ADC_IRQ_CURRENT_UVW,
	ADC_IRQ_CURRENT_XYZ,
	ADC_IRQ_VOLTAGE_DC,
	ADC_IRQ_TEMPERATURE_UVW,
	ADC_IRQ_TEMPERATURE_XYZ,
	ADC_IRQ_TEMPERATURE_CASE,

	ADC_IRQ_COUNT
};


/// @}
} // namespace adc


} // namespace mcu


