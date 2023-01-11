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


#include "F28x_Project.h"
#include "device.h"

#include "mcu_f2837xd/system/mcu_system.h"
#include "mcu_f2837xd/ipc/mcu_ipc.h"
#include "mcu_f2837xd/chrono/mcu_chrono.h"
#include "mcu_f2837xd/spi/mcu_spi.h"
#include "mcu_f2837xd/dac/mcu_dac.h"

#include "mcu_f2837xd/can/mcu_can.h"
#include "ucanopen/server/ucanopen_server.h"
#include "ucanopen/tests/ucanopen_tests.h"

#include "sys/syslog/syslog.h"
#include "clocktasks/clocktasks_cpu2.h"

#include "bsp_launchxl_f28379d/bsp_launchxl_f28379d.h"


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
 * @param None
 * @return None
 */
void main()
{
	/*##########*/
	/*# SYSTEM #*/
	/*##########*/
	mcu::initDevice();
	mcu::ipc::flags::cpu2Booted.local.set();

	bsp::initLedRed(mcu::gpio::MasterCore::Cpu2);
	bsp::ledRed.reset();

/*############################################################################*/
	/*##########*/
	/*# SYSLOG #*/
	/*##########*/
	SysLog::IpcFlags syslogIpcFlags =
	{
		.ipcResetErrorsWarnings = mcu::ipc::Flag(10, mcu::ipc::Mode::Dualcore),
		.ipcAddMessage = mcu::ipc::Flag(11, mcu::ipc::Mode::Dualcore),
		.ipcPopMessage = mcu::ipc::Flag(12, mcu::ipc::Mode::Dualcore)
	};
	SysLog::init(syslogIpcFlags);

/*############################################################################*/
	/*#########*/
	/*# CLOCK #*/
	/*#########*/
	mcu::chrono::SystemClock::init();
	mcu::chrono::SystemClock::registerTask(taskToggleLed, 2000);

	mcu::chrono::HighResolutionClock::init(100);
	//mcu::chronoHighResolutionClock::registerInterruptHandler(onSystickInterrupt);
	mcu::chrono::HighResolutionClock::start();

/*############################################################################*/
	/*#################*/
	/*# POWERUP DELAY #*/
	/*#################*/
	for (size_t i = 0; i < 10; ++i)
	{
		bsp::ledRed.toggle();
		mcu::delay_us(100000);
	}

/*############################################################################*/
	/*#######*/
	/*# CAN #*/
	/*#######*/
	mcu::can::Module<mcu::can::Peripheral::CanB> canB(
			mcu::gpio::Config(17, GPIO_17_CANRXB),
			mcu::gpio::Config(12, GPIO_12_CANTXB),
			mcu::can::Bitrate::Bitrate125K,
			mcu::can::Mode::Normal);

	ucanopen::IpcFlags canIpcFlags =
	{
		.rpdo1Received = mcu::ipc::Flag(4, mcu::ipc::Mode::Dualcore),
		.rpdo2Received = mcu::ipc::Flag(5, mcu::ipc::Mode::Dualcore),
		.rpdo3Received = mcu::ipc::Flag(6, mcu::ipc::Mode::Dualcore),
		.rpdo4Received = mcu::ipc::Flag(7, mcu::ipc::Mode::Dualcore),
		.rsdoReceived = mcu::ipc::Flag(8, mcu::ipc::Mode::Dualcore),
		.tsdoReady = mcu::ipc::Flag(9, mcu::ipc::Mode::Dualcore)
	};
	ucanopen::tests::Server<mcu::can::Peripheral::CanB, mcu::ipc::Mode::Dualcore, mcu::ipc::Role::Primary> canServer(
			ucanopen::NodeId(0x1), &canB, canIpcFlags);

/*############################################################################*/
	mcu::ipc::flags::cpu2PeripheryConfigured.local.set();
	mcu::ipc::flags::cpu1PeripheryConfigured.remote.wait();

/*############################################################################*/
	mcu::chrono::SystemClock::reset();
	mcu::enableMaskableInterrupts();
	mcu::enableDebugEvents();

	canServer.enable();

	while (true)
	{
		SysLog::processIpcSignals();
		mcu::chrono::SystemClock::runTasks();
		canServer.run();
	}
}

















