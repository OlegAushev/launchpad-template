///
#define FIRMWARE_VERSION_STRDEF "v22.08"
#define FIRMWARE_VERSION_NUMDEF 2207

#include "F28x_Project.h"
#include "device.h"
#include <new>

#include "emb/emb_profiler/emb_profiler.h"
#include "profiler/profiler.h"

#include "mcu_c28x/system/mcu_system.h"
#include "mcu_c28x/ipc/mcu_ipc.h"
#include "mcu_c28x/cputimers/mcu_cputimers.h"
#include "mcu_c28x/adc/mcu_adc.h"
#include "mcu_c28x/can/mcu_can.h"
#include "mcu_c28x/spi/mcu_spi.h"
#include "mcu_c28x/support/mcu_support.h"

#include "sys/syslog/syslog.h"
#include "sys/sysinfo/sysinfo.h"
#include "clocktasks/cpu1clocktasks.h"

#include "mcu_c28x/sci/mcu_sci.h"
#include "cli/cli_server.h"
#include "cli/shell/cli_shell.h"

#ifdef TEST_BUILD
#include "emb/emb_testrunner/emb_testrunner.h"
#endif


/* ========================================================================== */
/* ============================ SYSTEM INFO ================================= */
/* ========================================================================== */
const char* sys::DEVICE_NAME = "LaunchPad template project";
const char* sys::DEVICE_NAME_SHORT = "C28x";
const char* sys::FIRMWARE_VERSION = FIRMWARE_VERSION_STRDEF;
const uint32_t sys::FIRMWARE_VERSION_NUM = FIRMWARE_VERSION_NUMDEF;

#if defined(TEST_BUILD)
const char* sys::BUILD_CONFIGURATION = "TEST";
#elif defined(DEBUG)
const char* sys::BUILD_CONFIGURATION = "DEBUG";
#else
const char* sys::BUILD_CONFIGURATION = "RELEASE";
#endif
const char* sys::BUILD_CONFIGURATION_SHORT = sys::BUILD_CONFIGURATION;


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

#ifdef DUALCORE
	mcu::configureLaunchPadLeds(GPIO_CORE_CPU1, GPIO_CORE_CPU2);
	mcu::turnLedOff(mcu::LED_BLUE);
#else
	mcu::configureLaunchPadLeds(GPIO_CORE_CPU1, GPIO_CORE_CPU1);
	mcu::turnLedOff(mcu::LED_BLUE);
	mcu::turnLedOff(mcu::LED_RED);
#endif

#ifdef TEST_BUILD
	RUN_TESTS();
#endif

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
	Syslog::addMessage(sys::Message::DEVICE_CPU1_BOOT_SUCCESS);

// BEGIN of CPU1 PERIPHERY CONFIGURATION and OBJECTS CREATION
/*############################################################################*/
	/*#########*/
	/*# CLOCK #*/
	/*#########*/
	mcu::SystemClock::init();
	mcu::HighResolutionClock::init(1000000);
	mcu::HighResolutionClock::start();
	emb::DurationLogger_us::init(mcu::HighResolutionClock::now);
	emb::DurationLogger_clk::init(mcu::HighResolutionClock::counter);

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
	for (size_t i = 0; i < 10; ++i)
	{
		mcu::toggleLed(mcu::LED_BLUE);
		mcu::delay_us(100000);
	}

/*############################################################################*/
	mcu::enableMaskableInterrupts();	// cpu timer interrupt is used for timeouts
	mcu::enableDebugEvents();

/*############################################################################*/
	/*#############*/
	/*# BOOT CPU2 #*/
	/*#############*/
#ifdef DUALCORE
	mcu::bootCpu2();
	Syslog::addMessage(sys::Message::DEVICE_CPU2_BOOT);
	mcu::waitForRemoteIpcFlag(CPU2_BOOTED);
	Syslog::addMessage(sys::Message::DEVICE_CPU2_BOOT_SUCCESS);
#endif

/*############################################################################*/
	/*#######*/
	/*# ADC #*/
	/*#######*/
	mcu::AdcConfig adcConfig =
	{
		.sampleWindow_ns = 200,
	};

	mcu::Adc adc(adcConfig);

/*####################################################################################################################*/
	/*###############*/
	/*# CLOCK TASKS #*/
	/*###############*/
	mcu::SystemClock::setTaskPeriod(0, 1000);
	mcu::SystemClock::registerTask(0, taskToggleLed);

	mcu::SystemClock::setWatchdogPeriod(1000);
	mcu::SystemClock::registerWatchdogTask(taskWatchdogTimeout);

/*####################################################################################################################*/
#ifdef DUALCORE
	mcu::waitForRemoteIpcFlag(CPU2_PERIPHERY_CONFIGURED);
	Syslog::addMessage(sys::Message::DEVICE_CPU2_READY);
#endif

/*####################################################################################################################*/
	/*###################*/
	/*# ADC PREPARATION #*/
	/*###################*/
	adc.enableInterrupts();
	mcu::delay_us(100);	// wait for pending ADC INTs (after ADC calibrating) be served

/*####################################################################################################################*/
	mcu::SystemClock::reset();
#ifdef DUALCORE
	mcu::setLocalIpcFlag(CPU1_PERIPHERY_CONFIGURED);
#endif
	Syslog::addMessage(sys::Message::DEVICE_CPU1_READY);

/*####################################################################################################################*/
	//mcu::SystemClock::enableWatchdog();
	Syslog::addMessage(sys::Message::DEVICE_READY);

	while (true)
	{
		Syslog::processIpcSignals();
		mcu::SystemClock::runTasks();
	}
}















