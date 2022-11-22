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
struct ChannelName
{
	enum Type
	{
		AdcCurrentPhaseU,
		AdcCurrentPhaseV,
		AdcCurrentPhaseW,
		AdcCurrentPhaseX,
		AdcCurrentPhaseY,
		AdcCurrentPhaseZ,

		AdcVoltageDC,

		AdcTemperaturePhaseU,
		AdcTemperaturePhaseV,
		AdcTemperaturePhaseW,
		AdcTemperaturePhaseX,
		AdcTemperaturePhaseY,
		AdcTemperaturePhaseZ,
		AdcTemperatureCase,

		AdcChannelCount
	};
};


/// ADC interrupt request source (application-specific).
struct IrqName
{
	enum Type
	{
		AdcIrqCurrentUVW,
		AdcIrqCurrentXYZ,
		AdcIrqVoltageDC,
		AdcIrqTemperatureUVW,
		AdcIrqTemperatureXYZ,
		AdcIrqTemperatureCase,

		AdcIrqCount
	};
};


/// @}
} // namespace adc


} // namespace mcu


