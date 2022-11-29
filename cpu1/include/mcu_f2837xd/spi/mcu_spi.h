/**
 * @defgroup mcu_spi SPI
 * @ingroup mcu
 *
 * @file mcu_spi.h
 * @ingroup mcu mcu_spi
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
#include "../gpio/mcu_gpio.h"
#include "emb/emb_core.h"


namespace mcu {


namespace spi {
/// @addtogroup mcu_spi
/// @{


/// SPI modules
SCOPED_ENUM_DECLARE_BEGIN(Peripheral)
{
	SpiA,
	SpiB,
	SpiC
}
SCOPED_ENUM_DECLARE_END(Peripheral)


/// SPI protocol
SCOPED_ENUM_DECLARE_BEGIN(Protocol)
{
	Pol0Pha0 = SPI_PROT_POL0PHA0,	//!< Mode 0. Polarity 0, phase 0. Rising edge without delay
	Pol0Pha1 =  SPI_PROT_POL0PHA1,	//!< Mode 1. Polarity 0, phase 1. Rising edge with delay.
	Pol1Pha0 = SPI_PROT_POL1PHA0,	//!< Mode 2. Polarity 1, phase 0. Falling edge without delay.
	Pol1Pha1 = SPI_PROT_POL1PHA1	//!< Mode 3. Polarity 1, phase 1. Falling edge with delay.
}
SCOPED_ENUM_DECLARE_END(Protocol)


/// SPI mode
SCOPED_ENUM_DECLARE_BEGIN(Mode)
{
	Slave = SPI_MODE_SLAVE,			//!< SPI slave
	Master = SPI_MODE_MASTER,		//!< SPI master
	SlaveNoTalk = SPI_MODE_SLAVE_OD,	//!< SPI slave w/ output (TALK) disabled
	MasterNoTalk = SPI_MODE_MASTER_OD	//!< SPI master w/ output (TALK) disabled
}
SCOPED_ENUM_DECLARE_END(Mode)


/// SPI bitrates
SCOPED_ENUM_DECLARE_BEGIN(Bitrate)
{
	Bitrate1M = 1000000,
	Bitrate12M5 = 12500000,
}
SCOPED_ENUM_DECLARE_END(Bitrate)


/// SPI word length
SCOPED_ENUM_DECLARE_BEGIN(WordLen)
{
	Word8Bit = 8,
	Word16Bit = 16
}
SCOPED_ENUM_DECLARE_END(WordLen)


/**
 * @brief SPI unit config.
 */
struct Configuration
{
	Protocol protocol;
	Mode mode;
	Bitrate bitrate;
	WordLen wordLen;
	uint16_t dataSize;
};


namespace impl {


/**
 * @brief SPI module implementation.
 */
struct Module
{
	uint32_t base;
	uint32_t pieRxIntNum;
	Module(uint32_t _base, uint32_t _pieRxIntNum)
		: base(_base), pieRxIntNum(_pieRxIntNum) {}
};


extern const uint32_t spiBases[3];
extern const uint32_t spiRxPieIntNums[3];


} // namespace impl


/**
 * @brief SPI unit class.
 */
template <Peripheral::enum_type Instance>
class Spi : public emb::c28x::interrupt_invoker<Spi<Instance> >, private emb::noncopyable
{
private:
	impl::Module m_module;
	WordLen m_wordLen;
public:
	/**
	 * @brief Initializes MCU SPI unit.
	 * @param mosiPin
	 * @param misoPin
	 * @param clkPin
	 * @param csPin
	 * @param conf
	 */
	Spi(const gpio::Configuration& mosiPin, const gpio::Configuration& misoPin,
			const gpio::Configuration& clkPin, const gpio::Configuration& csPin,
			const Configuration& conf)
		: emb::c28x::interrupt_invoker<Spi<Instance> >(this)
		, m_module(impl::spiBases[Instance], impl::spiRxPieIntNums[Instance])
	{
		assert((conf.dataSize >= 1) && (conf.dataSize <= 16));

		m_wordLen = conf.wordLen;

		SPI_disableModule(m_module.base);
		SPI_setConfig(m_module.base, DEVICE_LSPCLK_FREQ,
				static_cast<SPI_TransferProtocol>(conf.protocol.underlying_value()),
				static_cast<SPI_Mode>(conf.mode.underlying_value()),
				static_cast<uint32_t>(conf.bitrate.underlying_value()),
				static_cast<uint16_t>(conf.wordLen.underlying_value()));
		SPI_disableLoopback(m_module.base);
		SPI_setEmulationMode(m_module.base, SPI_EMULATION_FREE_RUN);

#ifdef CPU1
		_initPins(mosiPin, misoPin, clkPin, csPin);
#endif

		SPI_enableFIFO(m_module.base);
		SPI_setFIFOInterruptLevel(m_module.base, SPI_FIFO_TXEMPTY, static_cast<SPI_RxFIFOLevel>(conf.dataSize));
		SPI_enableModule(m_module.base);
	}

#ifdef CPU1
	/**
	 * @brief Transfers control over SPI unit to CPU2.
	 * @param mosiPin
	 * @param misoPin
	 * @param clkPin
	 * @param csPin
	 * @param csMode
	 * @return (none)
	 */
	static void transferControlToCpu2(const gpio::Configuration& mosiPin, const gpio::Configuration& misoPin,
			const gpio::Configuration& clkPin, const gpio::Configuration& csPin)
	{
		_initPins(mosiPin, misoPin, clkPin, csPin);
		GPIO_setMasterCore(mosiPin.no, GPIO_CORE_CPU2);
		GPIO_setMasterCore(misoPin.no, GPIO_CORE_CPU2);
		GPIO_setMasterCore(clkPin.no, GPIO_CORE_CPU2);
		if (csPin.valid)
		{
			GPIO_setMasterCore(csPin.no, GPIO_CORE_CPU2);
		}

		SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL6_SPI,
				static_cast<uint16_t>(Instance)+1, SYSCTL_CPUSEL_CPU2);
	}
#endif

	/**
	 * @brief Returns base of SPI-unit.
	 * @param (none)
	 * @return Base of SPI-unit.
	 */
	uint32_t base() const { return m_module.base; }

	/**
	 * @brief Enables loopback mode.
	 * @param (none)
	 * @return (none)
	 */
	void enableLoopback() const
	{
		SPI_disableModule(m_module.base);
		SPI_enableLoopback(m_module.base);
		SPI_enableModule(m_module.base);
	}

	/**
	 * @brief Retrieves received data.
	 * @param data - reference to destination data
	 * @return (none)
	 */
	template <typename T>
	void recv(T& data) const
	{
		switch (m_wordLen.native_value())
		{
		case WordLen::Word8Bit:
			uint16_t byte8[sizeof(T)*2];
			for (size_t i = 0; i < sizeof(T)*2; ++i)
			{
				byte8[i] = SPI_readDataBlockingFIFO(m_module.base) & 0x00FF;
			}
			emb::c28x::from_bytes<T>(data, byte8);
			break;

		case WordLen::Word16Bit:
			uint16_t byte16[sizeof(T)];
			for (size_t i = 0; i < sizeof(T); ++i)
			{
				byte16[i] = SPI_readDataBlockingFIFO(m_module.base);
			}
			memcpy(&data, byte16, sizeof(T));
			break;
		}
	}

	/**
	 * @brief Sends data.
	 * @param data - reference to source data
	 * @return (none)
	 */
	template <typename T>
	void send(const T& data) const
	{
		switch (m_wordLen.native_value())
		{
		case WordLen::Word8Bit:
			uint16_t byte8[sizeof(T)*2];
			emb::c28x::to_bytes<T>(byte8, data);
			for (size_t i = 0; i < sizeof(T)*2; ++i)
			{
				SPI_writeDataBlockingFIFO(m_module.base, byte8[i] << 8);
			}
			break;

		case WordLen::Word16Bit:
			uint16_t byte16[sizeof(T)];
			memcpy(byte16, &data, sizeof(T));
			for (size_t i = 0; i < sizeof(T); ++i)
			{
				SPI_writeDataBlockingFIFO(m_module.base, byte16[i]);
			}
			break;
		}
	}

	/**
	 * @brief Registers Rx-interrupt handler.
	 * @param handler - pointer to interrupt handler
	 * @return (none)
	 */
	void registerRxInterruptHandler(void (*handler)(void)) const
	{
		SPI_disableModule(m_module.base);
		Interrupt_register(m_module.pieRxIntNum, handler);
		SPI_enableInterrupt(m_module.base, SPI_INT_RXFF);
		SPI_enableModule(m_module.base);
	}

	/**
	 * @brief Enables Rx-interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableRxInterrupts() const
	{
		Interrupt_enable(m_module.pieRxIntNum);
	}

	/**
	 * @brief Disables Rx-interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableRxInterrupts() const
	{
		Interrupt_disable(m_module.pieRxIntNum);
	}

	/**
	 * @brief Acknowledges interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void acknowledgeRxInterrupt() const
	{
		SPI_clearInterruptStatus(m_module.base, SPI_INT_RXFF);
		Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
	}

	/**
	 * @brief Resets RX FIFO.
	 * @param (none)
	 * @return (none)
	 */
	void resetRxFifo() const
	{
		SPI_resetRxFIFO(m_module.base);
	}

	/**
	 * @brief Resets TX FIFO.
	 * @param (none)
	 * @return (none)
	 */
	void resetTxFifo() const
	{
		SPI_resetTxFIFO(m_module.base);
	}

protected:
	static void _initPins(const gpio::Configuration& mosiPin, const gpio::Configuration& misoPin,
			const gpio::Configuration& clkPin, const gpio::Configuration& csPin)
	{
		GPIO_setPinConfig(mosiPin.mux);
		//GPIO_setPadConfig(mosiPin.no, GPIO_PIN_TYPE_PULLUP);
		GPIO_setQualificationMode(mosiPin.no, GPIO_QUAL_ASYNC);

		GPIO_setPinConfig(misoPin.mux);
		//GPIO_setPadConfig(misoPin.no, GPIO_PIN_TYPE_PULLUP);
		GPIO_setQualificationMode(misoPin.no, GPIO_QUAL_ASYNC);

		GPIO_setPinConfig(clkPin.mux);
		//GPIO_setPadConfig(clkPin.no, GPIO_PIN_TYPE_PULLUP);
		GPIO_setQualificationMode(clkPin.no, GPIO_QUAL_ASYNC);

		if (csPin.valid)
		{
			GPIO_setPinConfig(csPin.mux);
			//GPIO_setPadConfig(csPin.no, GPIO_PIN_TYPE_PULLUP);
			GPIO_setQualificationMode(csPin.no, GPIO_QUAL_ASYNC);
		}
	}
};


/// @}
} // namespace spi


} // namespace mcu


