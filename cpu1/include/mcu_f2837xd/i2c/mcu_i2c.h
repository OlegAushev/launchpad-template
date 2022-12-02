/**
 * @defgroup mcu_i2c I2C
 * @ingroup mcu
 *
 * @file mcu_i2c.h
 * @ingroup mcu mcu_i2c
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
#include "../gpio/mcu_gpio.h"
#include "emb/emb_core.h"


namespace mcu {


namespace i2c {
/// @addtogroup mcu_i2c
/// @{


/// I2C modules
SCOPED_ENUM_DECLARE_BEGIN(Peripheral)
{
	I2CA,
	I2CB
}
SCOPED_ENUM_DECLARE_END(Peripheral)


/// Count of bits per data word.
SCOPED_ENUM_DECLARE_BEGIN(BitCount)
{
	BC1 = I2C_BITCOUNT_1,
	BC2 = I2C_BITCOUNT_2,
	BC3 = I2C_BITCOUNT_3,
	BC4 = I2C_BITCOUNT_4,
	BC5 = I2C_BITCOUNT_5,
	BC6 = I2C_BITCOUNT_6,
	BC7 = I2C_BITCOUNT_7,
	BC8 = I2C_BITCOUNT_8
}
SCOPED_ENUM_DECLARE_END(BitCount)


/// I2C clock duty cycle.
SCOPED_ENUM_DECLARE_BEGIN(DutyCycle)
{
	DC33 = I2C_DUTYCYCLE_33,
	DC50 = I2C_DUTYCYCLE_50
}
SCOPED_ENUM_DECLARE_END(DutyCycle)


/**
 * @brief I2C module config.
 */
struct Configuration
{
	uint32_t bitrate;
	BitCount bitCount;
	DutyCycle dutyCycle;
	uint16_t slaveAddr;
};


namespace impl {


/**
 * @brief I2C module implementation
 */
struct Module
{
	uint32_t base;
	Module(uint32_t _base) : base(_base) {}
};


extern const uint32_t i2cBases[2];


} // namespace impl


/**
 * @brief I2C unit class.
 */
template <Peripheral::enum_type Instance>
class Module : public emb::c28x::interrupt_invoker<Module<Instance> >, private emb::noncopyable
{
private:
	impl::Module m_module;
public:
	/**
	 * @brief Initializes MCU I2C module.
	 * @param sdaPin - MCU I2C-SDA pin config
	 * @param sclPin - MCU I2C-SCL pin config
	 * @param conf - I2C config
	 */
	Module(const gpio::Configuration& sdaPin, const gpio::Configuration& sclPin, const i2c::Configuration& conf)
		: emb::c28x::interrupt_invoker<Module<Instance> >(this)
		, m_module(impl::i2cBases[Instance])
	{
#ifdef CPU1
		initPins(sdaPin, sclPin);
#endif
		I2C_disableModule(m_module.base);

		I2C_initMaster(m_module.base, mcu::sysclkFreq(), conf.bitrate,
				static_cast<I2C_DutyCycle>(conf.dutyCycle.underlying_value()));
		I2C_setBitCount(m_module.base, static_cast<I2C_BitCount>(conf.bitCount.underlying_value()));
		I2C_setSlaveAddress(m_module.base, conf.slaveAddr);
		I2C_setEmulationMode(m_module.base, I2C_EMULATION_FREE_RUN);

		I2C_disableFIFO(m_module.base);
		I2C_enableModule(m_module.base);
	}

#ifdef CPU1
	/**
	 * @brief Transfers control over I2C unit to CPU2.
	 * @param sdaPin - MCU I2C-SDA pin config
	 * @param sclPin - MCU I2C-SCL pin config
	 * @return (none)
	 */
	static void transferControlToCpu2(const gpio::Configuration& sdaPin, const gpio::Configuration& sclPin)
	{
		initPins(sdaPin, sclPin);
		GPIO_setMasterCore(sdaPin.no, GPIO_CORE_CPU2);
		GPIO_setMasterCore(sclPin.no, GPIO_CORE_CPU2);

		SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL7_I2C, static_cast<uint16_t>(Module)+1, SYSCTL_CPUSEL_CPU2);
	}
#endif

	/**
	 * @brief Returns base of I2C-unit.
	 * @param (none)
	 * @return Base of I2C-unit.
	 */
	uint32_t base() const { return m_module.base; }

	/**
	 * @brief Sets slave address.
	 * @param slaveAddr - slave address
	 * @return (none)
	 */
	void setSlaveAddress(uint16_t slaveAddr) { I2C_setSlaveAddress(module.base, slaveAddr); }

	/**
	 * @brief Enables unit.
	 * @param (none)
	 * @return (none)
	 */
	void enable() { I2C_enableModule(m_module.base); }

	/**
	 * @brief Disables unit.
	 * @param (none)
	 * @return (none)
	 */
	void disable() { I2C_disableModule(m_module.base); }

protected:
#ifdef CPU1
	static void initPins(const gpio::Configuration& sdaPin, const gpio::Configuration& sclPin)
	{
		GPIO_setPadConfig(sdaPin.no, GPIO_PIN_TYPE_PULLUP);
		GPIO_setQualificationMode(sdaPin.no, GPIO_QUAL_ASYNC);
		GPIO_setPinConfig(sdaPin.mux);

		GPIO_setPadConfig(sclPin.no, GPIO_PIN_TYPE_PULLUP);
		GPIO_setQualificationMode(sclPin.no, GPIO_QUAL_ASYNC);
		GPIO_setPinConfig(sclPin.mux);
	}
#endif
};


/// @}
} // namespace i2c


} // namespace mcu


