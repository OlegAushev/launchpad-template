
///
#define FIRMWARE_VERSION_STRDEF "v22.08.2"
#define FIRMWARE_VERSION_NUMDEF 22082

#include "../auto-generated/git_version.h"

#include "F28x_Project.h"
#include "device.h"
#include <new>

#include "emb/emb_profiler/emb_profiler.h"
#include "profiler/profiler.h"

#include "mcu_f2837xd/system/mcu_system.h"
#include "mcu_f2837xd/ipc/mcu_ipc.h"
#include "mcu_f2837xd/chrono/mcu_chrono.h"
#include "mcu_f2837xd/adc/mcu_adc.h"
#include "mcu_f2837xd/can/mcu_can.h"
#include "mcu_f2837xd/dac/mcu_dac.h"

#include "mcu_f2837xd/spi/mcu_spi.h"


#include "sys/syslog/syslog.h"
#include "sys/sysinfo/sysinfo.h"
#include "clocktasks/clocktasks_cpu1.h"

#include "mcu_f2837xd/sci/mcu_sci.h"
#include "cli/cli_server.h"
#include "cli/shell/cli_shell.h"

#include "bsp_launchxl_f28379d/bsp_launchxl_f28379d_def.h"
#include "bsp_launchxl_f28379d/leds/leds.h"

#include "tests/tests.h"


/* ========================================================================== */
/* ============================ SYSTEM INFO ================================= */
/* ========================================================================== */
const char* sys::DEVICE_NAME = "LaunchPad template project";
const char* sys::DEVICE_NAME_SHORT = "C28x";
const char* sys::FIRMWARE_VERSION = GIT_DESCRIBE;
const uint32_t sys::FIRMWARE_VERSION_NUM = GIT_COMMIT_NUM;

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
	mcu::Sci<mcu::SCIB> sciB(mcu::gpio::Config(bsp::j1_sciB_rxPin, bsp::j1_sciB_rxPinMux),
			mcu::gpio::Config(bsp::j1_sciB_txPin, bsp::j1_sciB_txPinMux),
			sciBConfig);

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
	cli::print_blocking(sys::FIRMWARE_VERSION);
	cli::print_blocking(CLI_COLOR_OFF);
	cli::nextline_blocking();
	cli::print_blocking("CPU1 has booted successfully");

/*############################################################################*/
	/*#######*/
	/*# BSP #*/
	/*#######*/
	cli::nextline_blocking();
	cli::print_blocking("initialize bsp... ");

#ifdef DUALCORE
	mcu::configureLaunchPadLeds(GPIO_CORE_CPU1, GPIO_CORE_CPU2);
	mcu::turnLedOff(mcu::LED_BLUE);
#else
	bsp::initLedBlue();
	bsp::initLedRed();
	bsp::ledBlue.reset();
	bsp::ledRed.reset();
#endif

	cli::print_blocking("done");

/*############################################################################*/
	/*#########*/
	/*# TESTS #*/
	/*#########*/
#ifdef ON_TARGET_TEST_BUILD
	emb::TestRunner::print = cli::print_blocking;
	emb::TestRunner::print_nextline = cli::nextline_blocking;

	cli::nextline_blocking();
	cli::print_blocking(CLI_COLOR_YELLOW "on-target testing build configuration is selected, run tests..." CLI_COLOR_OFF);
	cli::nextline_blocking();
	emb::run_tests();
#endif

/*############################################################################*/
	/*##########*/
	/*# SYSLOG #*/
	/*##########*/
	cli::nextline_blocking();
	cli::print_blocking("initialize syslog... ");

	Syslog::IpcFlags syslogIpcFlags =
	{
		.resetErrorsWarnings = mcu::ipc::Flag(10, mcu::ipc::IpcModeDualcore),
		.addMessage = mcu::ipc::Flag(11, mcu::ipc::IpcModeDualcore),
		.popMessage = mcu::ipc::Flag(12, mcu::ipc::IpcModeDualcore)
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

	mcu::chrono::SystemClock::init();
	mcu::chrono::HighResolutionClock::init(1000000);
	mcu::chrono::HighResolutionClock::start();
	emb::DurationLogger_us::init(mcu::chrono::HighResolutionClock::now);
	emb::DurationLogger_clk::init(mcu::chrono::HighResolutionClock::counter);

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
		bsp::ledBlue.toggle();
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
	mcu::ipc::flags::cpu2Booted.remote.wait();
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

	mcu::adc::Config adcConfig =
	{
		.sampleWindow_ns = 200,
	};

	mcu::adc::Module adc(adcConfig);

	cli::print_blocking("done");

/*############################################################################*/
	/*#######*/
	/*# DAC #*/
	/*#######*/
	cli::nextline_blocking();
	cli::print_blocking("configure DAC... ");

	mcu::dac::Module<mcu::dac::Peripheral::DacA> dacA;
	mcu::dac::Module<mcu::dac::Peripheral::DacA> dacB;

	cli::print_blocking("done");

/*####################################################################################################################*/
	/*###############*/
	/*# CLOCK TASKS #*/
	/*###############*/
	cli::nextline_blocking();
	cli::print_blocking("register periodic tasks... ");

	mcu::chrono::SystemClock::registerTask(taskToggleLed, 1000);

	mcu::chrono::SystemClock::registerWatchdogTask(taskWatchdogTimeout, 1000);

	cli::print_blocking("done");

/*####################################################################################################################*/
#ifdef DUALCORE
	cli::nextline_blocking();
	cli::print_blocking("waiting for CPU2 periphery configured... ");

	mcu::ipc::flags::cpu2PeripheryConfigured.remote.wait();
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
	mcu::chrono::SystemClock::reset();
#ifdef DUALCORE
	mcu::ipc::flags::cpu1PeripheryConfigured.local.set();
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
		mcu::chrono::SystemClock::runTasks();
		cliServer.run();
	}
}















