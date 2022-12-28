/**
 * @defgroup mcu_dac DAC
 * @ingroup mcu
 *
 * @file mcu_dac.h
 * @ingroup mcu mcu_dac
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
#include "emb/emb_core.h"


namespace mcu {


namespace dac {
/// @addtogroup mcu_dac
/// @{


/// DAC modules
SCOPED_ENUM_DECLARE_BEGIN(Peripheral)
{
	DacA,
	DacB,
	DacC
}
SCOPED_ENUM_DECLARE_END(Peripheral)


namespace impl {


/**
 * @brief DAC module implementation.
 */
struct Module
{
	uint32_t base;
	Module(uint32_t base_) : base(base_) {}
};


extern const uint32_t dacBases[3];


} // namespace impl


/**
 * @brief DAC input class.
 */
class Input
{
private:
	uint16_t _tag : 4;
	uint16_t _value : 12;
public:
	Input()
		: _tag(0)
		, _value(0)
	{}

	explicit Input(uint16_t value)
		: _tag(0)
		, _value(value & 0x0FFF)
	{}

	Input(uint16_t value, Peripheral peripheral)
		: _tag(static_cast<uint16_t>(peripheral.underlying_value()))
		, _value(value & 0x0FFF)
	{}

	uint16_t value() const { return _value; }
	uint16_t tag() const { return _tag; }
};


/**
 * @brief DAC unit class.
 */
template <Peripheral::enum_type Instance>
class Module : public emb::c28x::interrupt_invoker<Module<Instance> >, private emb::noncopyable
{
private:
	impl::Module _module;
public:
	/**
	 * @brief Initializes MCU DAC module.
	 * @param (none)
	 */
	Module()
		: emb::c28x::interrupt_invoker<Module<Instance> >(this)
		, _module(impl::dacBases[Instance])
	{
		DAC_setReferenceVoltage(_module.base, DAC_REF_ADC_VREFHI);
		DAC_enableOutput(_module.base);
		DAC_setShadowValue(_module.base, 0);
		mcu::delay_us(10);	// Delay for buffered DAC to power up
	}

	/**
	 * @brief Starts DAC.
	 * @param value - value to be converted.
	 * @return (none)
	 */
	void convert(Input input)
	{
		DAC_setShadowValue(_module.base, input.value());
	}
};


/// @}
} // namespace dac


} // namespace mcu


