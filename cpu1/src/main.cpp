/**
 * @file main.cpp
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "../auto-generated/git_version.h"

#include "F28x_Project.h"
#include "device.h"
#include <new>

#include "mcu_f2837xd/system/mcu_system.h"
#include "mcu_f2837xd/ipc/mcu_ipc.h"
#include "mcu_f2837xd/chrono/mcu_chrono.h"
#include "mcu_f2837xd/adc/mcu_adc.h"
#include "mcu_f2837xd/dac/mcu_dac.h"


#include "sys/syslog/syslog.h"
#include "sys/sysinfo/sysinfo.h"
#include "clocktasks/clocktasks_cpu1.h"

#include "mcu_f2837xd/sci/mcu_sci.h"
#include "cli/cli_server.h"
#include "cli/shell/cli_shell.h"

#include "mcu_f2837xd/can/mcu_can.h"
#include "ucanopen/server/ucanopen_server.h"
#include "ucanopen/tests/ucanopen_tests.h"

#include "bsp_launchxl_f28379d/bsp_launchxl_f28379d.h"
#include "emb/emb_profiler/emb_profiler.h"
#include "tests/tests.h"


/* ========================================================================== */
/* ============================ SYSTEM INFO ================================= */
/* ========================================================================== */
const char* SysInfo::deviceName = "LaunchPad template project";
const char* SysInfo::deviceNameShort = "C28x";
const char* SysInfo::firmwareVersion = GIT_DESCRIBE;
const uint32_t SysInfo::firmwareVersionNum = GIT_COMMIT_NUM;

#if defined(ON_TARGET_TEST_BUILD)
const char* SysInfo::buildConfiguration = "TEST";
const char* SysInfo::buildConfigurationShort = "TEST";
#elif defined(DEBUG)
const char* SysInfo::buildConfiguration = "DEBUG";
const char* SysInfo::buildConfigurationShort = "DBG";
#else
const char* SysInfo::buildConfiguration = "RELEASE";
const char* SysInfo::buildConfigurationShort = "RLS";
#endif


/* ========================================================================== */
/* ================================ MAIN ==================================== */
/* ========================================================================== */
/**
 * @brief main()
 */
void main()
{
	/*##########*/
	/*# SYSTEM #*/
	/*##########*/
	mcu::initDevice();
	MemCfg_setGSRAMMasterSel(MEMCFG_SECT_GS7, MEMCFG_GSRAMMASTER_CPU1);	// configs are placed here
	MemCfg_setGSRAMMasterSel(MEMCFG_SECT_GS8, MEMCFG_GSRAMMASTER_CPU2);	// CPU2 .bss is placed here
	MemCfg_setGSRAMMasterSel(MEMCFG_SECT_GS9, MEMCFG_GSRAMMASTER_CPU1);	// CPU1 to CPU2 data is placed here
	MemCfg_setGSRAMMasterSel(MEMCFG_SECT_GS10, MEMCFG_GSRAMMASTER_CPU2);	// CPU2 to CPU1 data is placed here

/*############################################################################*/
	/*#############*/
	/*# SCI & CLI #*/
	/*#############*/
	mcu::sci::Config sciBConf =
	{
		.baudrate = mcu::sci::Baudrate::Baudrate9600,
		.wordLen = mcu::sci::WordLen::Word8Bit,
		.stopBits = mcu::sci::StopBits::One,
		.parityMode = mcu::sci::ParityMode::None,
		.autoBaudMode = mcu::sci::AutoBaudMode::Disabled,
	};
	mcu::sci::Module<mcu::sci::Peripheral::SciB> sciB(
			mcu::gpio::Config(bsp::j1_sciB_rxPin, bsp::j1_sciB_rxPinMux),
			mcu::gpio::Config(bsp::j1_sciB_txPin, bsp::j1_sciB_txPinMux),
			sciBConf);

	cli::Server cliServer("launchpad", &sciB, NULL, NULL);
	cli::Shell::init();
	cliServer.registerExecCallback(cli::Shell::exec);
	cli::nextline_blocking();
	cli::nextline_blocking();
	cli::nextline_blocking();
	cli::print_blocking(CLI_WELCOME_STRING);
	cli::nextline_blocking();
	cli::print_blocking(CLI_COLOR_GREEN);
	cli::print_blocking("launchpad-template | ");
	cli::print_blocking(SysInfo::firmwareVersion);
	cli::print_blocking(CLI_COLOR_OFF);
	cli::nextline_blocking();
	cli::print_blocking("CPU1 boot... success.");

/*############################################################################*/
	/*#######*/
	/*# BSP #*/
	/*#######*/
	cli::nextline_blocking();
	cli::print_blocking("Initializing bsp... ");

#ifdef DUALCORE
	bsp::initLedBlue(mcu::gpio::MasterCore::Cpu1);
	bsp::initLedRed(mcu::gpio::MasterCore::Cpu2);
	bsp::ledBlue.reset();
#else
	bsp::initLedBlue();
	bsp::initLedRed();
	bsp::ledBlue.reset();
	bsp::ledRed.reset();
#endif

	cli::print_blocking("done.");

/*############################################################################*/
	/*#########*/
	/*# TESTS #*/
	/*#########*/
#ifdef ON_TARGET_TEST_BUILD
	emb::TestRunner::print = cli::print_blocking;
	emb::TestRunner::print_nextline = cli::nextline_blocking;

	cli::nextline_blocking();
	cli::print_blocking(CLI_COLOR_YELLOW "on-target testing build configuration selected, running tests..." CLI_COLOR_OFF);
	cli::nextline_blocking();
	emb::run_tests();
#endif

/*############################################################################*/
	/*##########*/
	/*# SYSLOG #*/
	/*##########*/
	cli::nextline_blocking();
	cli::print_blocking("Initializing syslog... ");

	SysLog::IpcFlags syslogIpcFlags =
	{
		.ipcResetErrorsWarnings = mcu::ipc::Flag(10, mcu::ipc::Mode::Dualcore),
		.ipcAddMessage = mcu::ipc::Flag(11, mcu::ipc::Mode::Dualcore),
		.ipcPopMessage = mcu::ipc::Flag(12, mcu::ipc::Mode::Dualcore)
	};
	SysLog::init(syslogIpcFlags);
	SysLog::addMessage(sys::Message::DeviceCpu1Booting);
	SysLog::addMessage(sys::Message::DeviceCpu1BootSuccess);

	cli::print_blocking("done.");

// BEGIN of CPU1 PERIPHERY CONFIGURATION and OBJECTS CREATION
/*############################################################################*/
	/*#########*/
	/*# CLOCK #*/
	/*#########*/
	cli::nextline_blocking();
	cli::print_blocking("Configuring system clock and high resolution clock... ");

	mcu::chrono::SystemClock::init();
	mcu::chrono::HighResolutionClock::init(1000000);
	mcu::chrono::HighResolutionClock::start();
	emb::DurationLogger_us::init(mcu::chrono::HighResolutionClock::now);
	emb::DurationLogger_clk::init(mcu::chrono::HighResolutionClock::counter);

	cli::print_blocking("done.");

	// ALL PERFORMANCE TESTS MUST BE PERFORMED AFTER THIS POINT!!!

	/*-------------------------*/
	/* PERFORMANCE TESTS BEGIN */
	/*-------------------------*/

	/*-----------------------*/
	/* PERFORMANCE TESTS END */
	/*-----------------------*/

/*############################################################################*/
	/*#################*/
	/*# POWERUP DELAY #*/
	/*#################*/
	cli::nextline_blocking();
	cli::print_blocking("Startup delay... ");

	for (size_t i = 0; i < 10; ++i)
	{
		bsp::ledBlue.toggle();
		mcu::delay_us(100000);
	}

	cli::print_blocking("done.");

/*############################################################################*/
	cli::nextline_blocking();
	cli::print_blocking("Enabling interrupts... ");

	mcu::enableMaskableInterrupts();	// cpu timer interrupt is used for timeouts
	mcu::enableDebugEvents();

	cli::print_blocking("done.");

/*############################################################################*/
	/*#############*/
	/*# BOOT CPU2 #*/
	/*#############*/
#ifdef DUALCORE

	cli::nextline_blocking();
	cli::print_blocking("Transferring control over CANB to CPU2... ");

	mcu::can::Module<mcu::can::Peripheral::CanB>::transferControlToCpu2(
			mcu::gpio::Config(17, GPIO_17_CANRXB),
			mcu::gpio::Config(12, GPIO_12_CANTXB)
	);

	cli::print_blocking("success.");

	cli::nextline_blocking();
	cli::print_blocking("CPU2 boot... ");

	mcu::bootCpu2();
	SysLog::addMessage(sys::Message::DeviceCpu2Booting);
	mcu::ipc::flags::cpu2Booted.remote.wait();
	SysLog::addMessage(sys::Message::DeviceCpu2BootSuccess);

	cli::print_blocking("success.");
#else
	cli::nextline_blocking();
	cli::print_blocking("CPU2 disabled. Skip CPU2 boot.");
#endif

/*############################################################################*/
	/*#######*/
	/*# ADC #*/
	/*#######*/
	cli::nextline_blocking();
	cli::print_blocking("Configuring ADC... ");

	mcu::adc::Config adcConfig =
	{
		.sampleWindow_ns = 200,
	};

	mcu::adc::Module adc(adcConfig);

	cli::print_blocking("done.");

/*############################################################################*/
	/*#######*/
	/*# DAC #*/
	/*#######*/
	cli::nextline_blocking();
	cli::print_blocking("Configuring DAC... ");

	mcu::dac::Module<mcu::dac::Peripheral::DacA> dacA;
	mcu::dac::Module<mcu::dac::Peripheral::DacB> dacB;

	cli::print_blocking("done.");

/*############################################################################*/
	/*#######*/
	/*# CAN #*/
	/*#######*/
	cli::nextline_blocking();
	cli::print_blocking("Configuring CAN... ");

	mcu::can::Module<mcu::can::Peripheral::CanB> canB(
			mcu::gpio::Config(17, GPIO_17_CANRXB),
			mcu::gpio::Config(12, GPIO_12_CANTXB),
			mcu::can::Bitrate::Bitrate125K,
			mcu::can::Mode::Normal);

#ifdef DUALCORE
	ucanopen::IpcFlags canIpcFlags =
	{
		.rpdo1Received = mcu::ipc::Flag(4, mcu::ipc::Mode::Dualcore),
		.rpdo2Received = mcu::ipc::Flag(5, mcu::ipc::Mode::Dualcore),
		.rpdo3Received = mcu::ipc::Flag(6, mcu::ipc::Mode::Dualcore),
		.rpdo4Received = mcu::ipc::Flag(7, mcu::ipc::Mode::Dualcore),
		.rsdoReceived = mcu::ipc::Flag(8, mcu::ipc::Mode::Dualcore),
		.tsdoReady = mcu::ipc::Flag(9, mcu::ipc::Mode::Dualcore)
	};
	ucanopen::IServer<mcu::can::Peripheral::CanB, mcu::ipc::Mode::Dualcore, mcu::ipc::Role::Secondary> canServer;
#else
	ucanopen::IpcFlags canIpcFlags =
	{
		.rpdo1Received = mcu::ipc::Flag(4, mcu::ipc::Mode::Singlecore),
		.rpdo2Received = mcu::ipc::Flag(5, mcu::ipc::Mode::Singlecore),
		.rpdo3Received = mcu::ipc::Flag(6, mcu::ipc::Mode::Singlecore),
		.rpdo4Received = mcu::ipc::Flag(7, mcu::ipc::Mode::Singlecore),
		.rsdoReceived = mcu::ipc::Flag(8, mcu::ipc::Mode::Singlecore),
		.tsdoReady = mcu::ipc::Flag(9, mcu::ipc::Mode::Singlecore)
	};
	ucanopen::tests::Server<mcu::can::Peripheral::CanB, mcu::ipc::Mode::Singlecore, mcu::ipc::Role::Primary> canServer(
			ucanopen::NodeId(0x1), &canB, canIpcFlags,
			ucanopen::tests::objectDictionary, ucanopen::tests::objectDictionaryLen);
#endif

	cli::print_blocking("done.");

/*####################################################################################################################*/
	/*###############*/
	/*# CLOCK TASKS #*/
	/*###############*/
	cli::nextline_blocking();
	cli::print_blocking("Registering periodic tasks... ");

	mcu::chrono::SystemClock::registerTask(taskToggleLed, 1000);

	mcu::chrono::SystemClock::registerWatchdogTask(taskWatchdogTimeout, 1000);

	cli::print_blocking("done.");

/*####################################################################################################################*/
#ifdef DUALCORE
	cli::nextline_blocking();
	cli::print_blocking("Waiting for CPU2 periphery configured... ");

	mcu::ipc::flags::cpu2PeripheryConfigured.remote.wait();
	SysLog::addMessage(sys::Message::DeviceCpu2Ready);

	cli::print_blocking("success.");
#endif

/*####################################################################################################################*/
	/*###################*/
	/*# ADC PREPARATION #*/
	/*###################*/
	adc.enableInterrupts();
	mcu::delay_us(100);	// wait for pending ADC INTs (after ADC calibrating) be served

/*####################################################################################################################*/
	mcu::chrono::SystemClock::reset();
#ifdef DUALCORE
	mcu::ipc::flags::cpu1PeripheryConfigured.local.set();
#endif
	SysLog::addMessage(sys::Message::DeviceCpu1Ready);

	cli::nextline_blocking();
	cli::print_blocking("CPU1 periphery successfully configured.");

/*####################################################################################################################*/
	//mcu::SystemClock::enableWatchdog();
	SysLog::addMessage(sys::Message::DeviceReady);

	cli::nextline_blocking();
	cli::print_blocking("Device ready!");

	canServer.enable();

	while (true)
	{
		SysLog::processIpcSignals();
		mcu::chrono::SystemClock::runTasks();
		cliServer.run();
		canServer.run();
	}
}















