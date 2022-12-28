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
SCOPED_ENUM_UT_DECLARE_BEGIN(Bitrate, uint32_t)
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
struct Config
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
	Module(uint32_t base_, uint32_t pieRxIntNum_)
		: base(base_), pieRxIntNum(pieRxIntNum_) {}
};


extern const uint32_t spiBases[3];
extern const uint32_t spiRxPieIntNums[3];


} // namespace impl


/**
 * @brief SPI unit class.
 */
template <Peripheral::enum_type Instance>
class Module : public emb::c28x::interrupt_invoker<Module<Instance> >, private emb::noncopyable
{
private:
	impl::Module _module;
	WordLen _wordLen;
public:
	/**
	 * @brief Initializes MCU SPI module.
	 * @param mosiPin
	 * @param misoPin
	 * @param clkPin
	 * @param csPin
	 * @param config
	 */
	Module(const gpio::Config& mosiPin, const gpio::Config& misoPin,
			const gpio::Config& clkPin, const gpio::Config& csPin,
			const Config& config)
		: emb::c28x::interrupt_invoker<Module<Instance> >(this)
		, _module(impl::spiBases[Instance], impl::spiRxPieIntNums[Instance])
	{
		assert((config.dataSize >= 1) && (config.dataSize <= 16));

		_wordLen = config.wordLen;

		SPI_disableModule(_module.base);
		SPI_setConfig(_module.base, DEVICE_LSPCLK_FREQ,
				static_cast<SPI_TransferProtocol>(config.protocol.underlying_value()),
				static_cast<SPI_Mode>(config.mode.underlying_value()),
				static_cast<uint32_t>(config.bitrate.underlying_value()),
				static_cast<uint16_t>(config.wordLen.underlying_value()));
		SPI_disableLoopback(_module.base);
		SPI_setEmulationMode(_module.base, SPI_EMULATION_FREE_RUN);

#ifdef CPU1
		_initPins(mosiPin, misoPin, clkPin, csPin);
#endif

		SPI_enableFIFO(_module.base);
		SPI_setFIFOInterruptLevel(_module.base, SPI_FIFO_TXEMPTY, static_cast<SPI_RxFIFOLevel>(config.dataSize));
		SPI_enableModule(_module.base);
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
	static void transferControlToCpu2(const gpio::Config& mosiPin, const gpio::Config& misoPin,
			const gpio::Config& clkPin, const gpio::Config& csPin)
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
	uint32_t base() const { return _module.base; }

	/**
	 * @brief Enables loopback mode.
	 * @param (none)
	 * @return (none)
	 */
	void enableLoopback()
	{
		SPI_disableModule(_module.base);
		SPI_enableLoopback(_module.base);
		SPI_enableModule(_module.base);
	}

	/**
	 * @brief Retrieves received data.
	 * @param data - reference to destination data
	 * @return (none)
	 */
	template <typename T>
	void recv(T& data)
	{
		switch (_wordLen.native_value())
		{
		case WordLen::Word8Bit:
			uint16_t byte8[sizeof(T)*2];
			for (size_t i = 0; i < sizeof(T)*2; ++i)
			{
				byte8[i] = SPI_readDataBlockingFIFO(_module.base) & 0x00FF;
			}
			emb::c28x::from_bytes<T>(data, byte8);
			break;

		case WordLen::Word16Bit:
			uint16_t byte16[sizeof(T)];
			for (size_t i = 0; i < sizeof(T); ++i)
			{
				byte16[i] = SPI_readDataBlockingFIFO(_module.base);
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
	void send(const T& data)
	{
		switch (_wordLen.native_value())
		{
		case WordLen::Word8Bit:
			uint16_t byte8[sizeof(T)*2];
			emb::c28x::to_bytes<T>(byte8, data);
			for (size_t i = 0; i < sizeof(T)*2; ++i)
			{
				SPI_writeDataBlockingFIFO(_module.base, byte8[i] << 8);
			}
			break;

		case WordLen::Word16Bit:
			uint16_t byte16[sizeof(T)];
			memcpy(byte16, &data, sizeof(T));
			for (size_t i = 0; i < sizeof(T); ++i)
			{
				SPI_writeDataBlockingFIFO(_module.base, byte16[i]);
			}
			break;
		}
	}

	/**
	 * @brief Registers Rx-interrupt handler.
	 * @param handler - pointer to interrupt handler
	 * @return (none)
	 */
	void registerRxInterruptHandler(void (*handler)(void))
	{
		SPI_disableModule(_module.base);
		Interrupt_register(_module.pieRxIntNum, handler);
		SPI_enableInterrupt(_module.base, SPI_INT_RXFF);
		SPI_enableModule(_module.base);
	}

	/**
	 * @brief Enables Rx-interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void enableRxInterrupts()
	{
		Interrupt_enable(_module.pieRxIntNum);
	}

	/**
	 * @brief Disables Rx-interrupts.
	 * @param (none)
	 * @return (none)
	 */
	void disableRxInterrupts()
	{
		Interrupt_disable(_module.pieRxIntNum);
	}

	/**
	 * @brief Acknowledges interrupt.
	 * @param (none)
	 * @return (none)
	 */
	void acknowledgeRxInterrupt()
	{
		SPI_clearInterruptStatus(_module.base, SPI_INT_RXFF);
		Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
	}

	/**
	 * @brief Resets RX FIFO.
	 * @param (none)
	 * @return (none)
	 */
	void resetRxFifo()
	{
		SPI_resetRxFIFO(_module.base);
	}

	/**
	 * @brief Resets TX FIFO.
	 * @param (none)
	 * @return (none)
	 */
	void resetTxFifo()
	{
		SPI_resetTxFIFO(_module.base);
	}

protected:
	static void _initPins(const gpio::Config& mosiPin, const gpio::Config& misoPin,
			const gpio::Config& clkPin, const gpio::Config& csPin)
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


