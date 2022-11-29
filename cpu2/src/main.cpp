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

#include "profiler/profiler.h"

#include "mcu_f2837xd/system/mcu_system.h"
#include "mcu_f2837xd/ipc/mcu_ipc.h"
#include "mcu_f2837xd/cputimers/mcu_cputimers.h"
#include "mcu_f2837xd/spi/mcu_spi.h"
#include "mcu_f2837xd/dac/mcu_dac.h"
#include "mcu_f2837xd/support/mcu_support.h"

#include "sys/syslog/syslog.h"
#include "clocktasks/clocktasks_cpu2.h"


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
	mcu::setLocalIpcFlag(CPU2_BOOTED);

	mcu::turnLedOff(mcu::LED_RED);

/*############################################################################*/
	/*##########*/
	/*# SYSLOG #*/
	/*##########*/
	Syslog::IpcFlags syslogIpcFlags =
	{
		.RESET_ERRORS_WARNINGS = mcu::IpcFlag(10),
		.ADD_MESSAGE = mcu::IpcFlag(11),
		.POP_MESSAGE = mcu::IpcFlag(12)
	};
	Syslog::init(syslogIpcFlags);

/*############################################################################*/
	/*#########*/
	/*# CLOCK #*/
	/*#########*/
	mcu::SystemClock::init();
	mcu::SystemClock::setTaskPeriod(0, 2000);
	mcu::SystemClock::registerTask(0, taskToggleLed);
	mcu::HighResolutionClock::init(100);
	//mcu::HighResolutionClock::registerInterruptHandler(onSystickInterrupt);
	mcu::HighResolutionClock::start();

/*############################################################################*/
	/*#################*/
	/*# POWERUP DELAY #*/
	/*#################*/
	for (size_t i = 0; i < 10; ++i)
	{
		mcu::toggleLed(mcu::LED_RED);
		mcu::delay_us(100000);
	}

/*############################################################################*/
	mcu::setLocalIpcFlag(CPU2_PERIPHERY_CONFIGURED);
	mcu::waitForRemoteIpcFlag(CPU1_PERIPHERY_CONFIGURED);

/*############################################################################*/
	mcu::SystemClock::reset();
	mcu::enableMaskableInterrupts();
	mcu::enableDebugEvents();

	while (true)
	{
		Syslog::processIpcSignals();
		mcu::SystemClock::runTasks();
	}
}

















