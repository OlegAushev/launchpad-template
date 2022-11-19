///
#define FIRMWARE_VERSION_STRDEF "v22.08.2"
#define FIRMWARE_VERSION_NUMDEF 22082

#include "F28x_Project.h"
#include "device.h"
#include <new>

#include "emb/emb_profiler/emb_profiler.h"
#include "profiler/profiler.h"

#include "mcu_f2837xd/system/mcu_system.h"
#include "mcu_f2837xd/ipc/mcu_ipc.h"
#include "mcu_f2837xd/cputimers/mcu_cputimers.h"
#include "mcu_f2837xd/adc/mcu_adc.h"
#include "mcu_f2837xd/can/mcu_can.h"
#include "mcu_f2837xd/spi/mcu_spi.h"
#include "mcu_f2837xd/support/mcu_support.h"

#include "sys/syslog/syslog.h"
#include "sys/sysinfo/sysinfo.h"
#include "clocktasks/clocktasks_cpu1.h"

#include "mcu_f2837xd/sci/mcu_sci.h"
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

/*############################################################################*/
	/*#############*/
	/*# SCI & CLI #*/
	/*#############*/
	mcu::SciConfig sciBConfig =
	{
		.baudrate = mcu::SCI_BAUDRATE_9600,
		.wordLen = mcu::SCI_WORD_8BIT,
		.stopBits = mcu::SCI_STOP_BIT_ONE,
		.parityMode = mcu::SCI_PARITY_NONE,
		.autoBaudMode = mcu::SCI_AUTO_BAUD_DISABLED,
	};
	mcu::Sci<mcu::SCIB> sciB(mcu::gpio::Config(19, GPIO_19_SCIRXDB),
			mcu::gpio::Config(18, GPIO_18_SCITXDB),
			sciBConfig);

	cli::Server cliServer("launchpad", &sciB, NULL, NULL);
	cli::Shell::init();
	cliServer.registerExecCallback(cli::Shell::exec);
	cli::nextline_blocking();
	cli::nextline_blocking();
	cli::nextline_blocking();
	cli::print_blocking(CLI_WELCOME_STRING);
	cli::nextline_blocking();
	cli::print_blocking("CPU1 has booted successfully");

/*############################################################################*/
#ifdef TEST_BUILD
	RUN_TESTS();
#endif

/*############################################################################*/
	/*##########*/
	/*# SYSLOG #*/
	/*##########*/
	cli::nextline_blocking();
	cli::print_blocking("initialize syslog... ");

	Syslog::IpcFlags syslogIpcFlags =
	{
		.RESET_ERRORS_WARNINGS = mcu::IpcFlag(10),
		.ADD_MESSAGE = mcu::IpcFlag(11),
		.POP_MESSAGE = mcu::IpcFlag(12)
	};
	Syslog::init(syslogIpcFlags);
	Syslog::addMessage(sys::Message::DEVICE_CPU1_BOOT_SUCCESS);

	cli::print_blocking("done");

// BEGIN of CPU1 PERIPHERY CONFIGURATION and OBJECTS CREATION
/*############################################################################*/
	/*#########*/
	/*# CLOCK #*/
	/*#########*/
	cli::nextline_blocking();
	cli::print_blocking("configure system clock and high resolution clock... ");

	mcu::SystemClock::init();
	mcu::HighResolutionClock::init(1000000);
	mcu::HighResolutionClock::start();
	emb::DurationLogger_us::init(mcu::HighResolutionClock::now);
	emb::DurationLogger_clk::init(mcu::HighResolutionClock::counter);

	cli::print_blocking("done");

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
	cli::print_blocking("startup delay... ");

	for (size_t i = 0; i < 10; ++i)
	{
		mcu::toggleLed(mcu::LED_BLUE);
		mcu::delay_us(100000);
	}

	cli::print_blocking("done");

/*############################################################################*/
	cli::nextline_blocking();
	cli::print_blocking("enable interrupts... ");

	mcu::enableMaskableInterrupts();	// cpu timer interrupt is used for timeouts
	mcu::enableDebugEvents();

	cli::print_blocking("done");

/*############################################################################*/
	/*#############*/
	/*# BOOT CPU2 #*/
	/*#############*/
#ifdef DUALCORE
	cli::nextline_blocking();
	cli::print_blocking("boot CPU2... ");

	mcu::bootCpu2();
	Syslog::addMessage(sys::Message::DEVICE_CPU2_BOOT);
	mcu::waitForRemoteIpcFlag(CPU2_BOOTED);
	Syslog::addMessage(sys::Message::DEVICE_CPU2_BOOT_SUCCESS);

	cli::print_blocking("success");
#else
	cli::nextline_blocking();
	cli::print_blocking("CPU2 is disabled. CPU2 boot skipped.");
#endif

/*############################################################################*/
	/*#######*/
	/*# ADC #*/
	/*#######*/
	cli::nextline_blocking();
	cli::print_blocking("configure ADC... ");

	mcu::AdcConfig adcConfig =
	{
		.sampleWindow_ns = 200,
	};

	mcu::Adc adc(adcConfig);

	cli::print_blocking("done");

/*####################################################################################################################*/
	/*###############*/
	/*# CLOCK TASKS #*/
	/*###############*/
	cli::nextline_blocking();
	cli::print_blocking("register periodic tasks... ");

	mcu::SystemClock::setTaskPeriod(0, 1000);
	mcu::SystemClock::registerTask(0, taskToggleLed);

	mcu::SystemClock::setWatchdogPeriod(1000);
	mcu::SystemClock::registerWatchdogTask(taskWatchdogTimeout);

	cli::print_blocking("done");

/*####################################################################################################################*/
#ifdef DUALCORE
	cli::nextline_blocking();
	cli::print_blocking("waiting for CPU2 periphery configured... ");

	mcu::waitForRemoteIpcFlag(CPU2_PERIPHERY_CONFIGURED);
	Syslog::addMessage(sys::Message::DEVICE_CPU2_READY);

	cli::print_blocking("success");
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

	cli::nextline_blocking();
	cli::print_blocking("CPU1 periphery has been successfully configured");

/*####################################################################################################################*/
	//mcu::SystemClock::enableWatchdog();
	Syslog::addMessage(sys::Message::DEVICE_READY);

	cli::nextline_blocking();
	cli::print_blocking("device is ready!");

	while (true)
	{
		Syslog::processIpcSignals();
		mcu::SystemClock::runTasks();
		cliServer.run();
	}
}















