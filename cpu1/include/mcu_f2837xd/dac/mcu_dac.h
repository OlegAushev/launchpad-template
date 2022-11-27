/**
 * @defgroup mcu_dac DAC
 * @ingroup mcu
 *
 * @file
 * @ingroup mcu mcu_dac
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
	Module(uint32_t _base) : base(_base) {}
};


extern const uint32_t dacBases[3];


} // namespace impl


/**
 * @brief DAC input class.
 */
class Input
{
private:
	uint16_t m_tag : 4;
	uint16_t m_value : 12;
public:
	Input()
		: m_tag(0)
		, m_value(0)
	{}

	explicit Input(uint16_t value)
		: m_tag(0)
		, m_value(value & 0x0FFF)
	{}

	Input(uint16_t value, Peripheral peripheral)
		: m_tag(static_cast<uint16_t>(peripheral.underlying_value()))
		, m_value(value & 0x0FFF)
	{}

	uint16_t value() const { return m_value; }
	uint16_t tag() const { return m_tag; }
};


/**
 * @brief DAC unit class.
 */
template <Peripheral::enum_type Instance>
class Module : public emb::c28x::interrupt_invoker<Module<Instance> >
{
private:
	impl::Module m_module;
private:
	Module(const Module& other);		// no copy constructor
	Module& operator=(const Module& other);	// no copy assignment operator
public:
	/**
	 * @brief Initializes MCU DAC unit.
	 * @param (none)
	 */
	Module()
		: emb::c28x::interrupt_invoker<Module<Instance> >(this)
		, m_module(impl::dacBases[Instance])
	{
		DAC_setReferenceVoltage(m_module.base, DAC_REF_ADC_VREFHI);
		DAC_enableOutput(m_module.base);
		DAC_setShadowValue(m_module.base, 0);
		mcu::delay_us(10);	// Delay for buffered DAC to power up
	}

	/**
	 * @brief Starts DAC.
	 * @param value - value to be converted.
	 * @return (none)
	 */
	void convert(Input input)
	{
		DAC_setShadowValue(m_module.base, input.value());
	}
};


/// @}
} // namespace dac


} // namespace mcu


