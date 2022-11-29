/**
 * @file mcu_adcchannels.h
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


