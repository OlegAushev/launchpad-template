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
SCOPED_ENUM_DECLARE_BEGIN(ChannelName)
{
	CurrentPhaseU,
	CurrentPhaseV,
	CurrentPhaseW,
	CurrentPhaseX,
	CurrentPhaseY,
	CurrentPhaseZ,

	VoltageDC,

	TemperaturePhaseU,
	TemperaturePhaseV,
	TemperaturePhaseW,
	TemperaturePhaseX,
	TemperaturePhaseY,
	TemperaturePhaseZ,
	TemperatureCase,

	Count
}
SCOPED_ENUM_DECLARE_END(ChannelName)


/// ADC interrupt request source (application-specific).
SCOPED_ENUM_DECLARE_BEGIN(IrqName)
{
	CurrentUVW,
	CurrentXYZ,
	VoltageDC,
	TemperatureUVW,
	TemperatureXYZ,
	TemperatureCase,

	Count
}
SCOPED_ENUM_DECLARE_END(IrqName)


/// @}
} // namespace adc


} // namespace mcu


