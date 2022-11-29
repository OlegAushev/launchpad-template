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

#include "sys/syslog/syslog.h"
#include "clocktasks/clocktasks_cpu2.h"

#include "bsp_launchxl_f28379d/bsp_launchxl_f28379d.h"


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
		.ipcResetErrorsWarnings = mcu::ipc::Flag(10, mcu::ipc::IpcMode::Dualcore),
		.ipcAddMessage = mcu::ipc::Flag(11, mcu::ipc::IpcMode::Dualcore),
		.ipcPopMessage = mcu::ipc::Flag(12, mcu::ipc::IpcMode::Dualcore)
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
	mcu::ipc::flags::cpu2PeripheryConfigured.local.set();
	mcu::ipc::flags::cpu1PeripheryConfigured.remote.wait();

/*############################################################################*/
	mcu::chrono::SystemClock::reset();
	mcu::enableMaskableInterrupts();
	mcu::enableDebugEvents();

	while (true)
	{
		SysLog::processIpcSignals();
		mcu::chrono::SystemClock::runTasks();
	}
}

















