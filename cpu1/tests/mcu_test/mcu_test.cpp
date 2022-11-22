///
#include "mcu_test.h"

///
///
///
void McuTest::GpioTest()
{
#ifdef _LAUNCHXL_F28379D
	mcu::gpio::Config ledBlueCfg(31, GPIO_31_GPIO31, mcu::gpio::PIN_OUTPUT, emb::ActiveHigh, mcu::gpio::PIN_STD, mcu::gpio::PIN_QUAL_SYNC, 1);
	mcu::gpio::Config ledRedCfg(34, GPIO_34_GPIO34, mcu::gpio::PIN_OUTPUT, emb::ActiveLow, mcu::gpio::PIN_STD, mcu::gpio::PIN_QUAL_SYNC, 1);

	mcu::gpio::Output ledBlue(ledBlueCfg);
	mcu::gpio::Output ledRed(ledRedCfg);

	mcu::turnLedOff(mcu::LED_BLUE);
	mcu::turnLedOff(mcu::LED_RED);

	EMB_ASSERT_EQUAL(ledBlue.read(), emb::PinActive);
	EMB_ASSERT_EQUAL(ledRed.read(), emb::PinInactive);

	ledBlue.set(emb::PinInactive);	// led - on
	ledRed.set(emb::PinInactive);	// led - off
	DEVICE_DELAY_US(100000);

	EMB_ASSERT_EQUAL(GPIO_readPin(31), 0);
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 1);
	EMB_ASSERT_EQUAL(ledBlue.read(), emb::PinInactive);
	EMB_ASSERT_EQUAL(ledRed.read(), emb::PinInactive);

	ledBlue.set(emb::PinActive);	// led - off
	ledRed.set(emb::PinActive);	// led - on
	DEVICE_DELAY_US(100000);

	EMB_ASSERT_EQUAL(GPIO_readPin(31), 1);
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 0);
	EMB_ASSERT_EQUAL(ledBlue.read(), emb::PinActive);
	EMB_ASSERT_EQUAL(ledRed.read(), emb::PinActive);

	mcu::turnLedOff(mcu::LED_BLUE);
	mcu::turnLedOff(mcu::LED_RED);
#elif defined(TEST_BUILD)
#warning "LAUNCHXL is required for full testing."
#endif

#ifdef _LAUNCHXL_F28379D
	mcu::gpio::Config outCfg(27, GPIO_27_GPIO27, mcu::gpio::PIN_OUTPUT, emb::ActiveHigh, mcu::gpio::PIN_STD, mcu::gpio::PIN_QUAL_SYNC, 1);
	mcu::gpio::Config in1Cfg(25, GPIO_25_GPIO25, mcu::gpio::PIN_INPUT, emb::ActiveHigh, mcu::gpio::PIN_STD, mcu::gpio::PIN_QUAL_SYNC, 1);
	mcu::gpio::Config in2Cfg(25, GPIO_25_GPIO25, mcu::gpio::PIN_INPUT, emb::ActiveLow, mcu::gpio::PIN_STD, mcu::gpio::PIN_QUAL_SYNC, 1);

	mcu::gpio::Output out(outCfg);
	mcu::gpio::Input in1(in1Cfg);
	mcu::gpio::InputDebouncer db1(in1, 10, 20, 30);

	EMB_ASSERT_EQUAL(in1.read(), emb::PinInactive);
	EMB_ASSERT_EQUAL(db1.state(), emb::PinInactive);
	EMB_ASSERT_TRUE(!db1.stateChanged());

	out.set(emb::PinActive);
	EMB_ASSERT_EQUAL(in1.read(), emb::PinActive);
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::PinInactive);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::PinActive);
	EMB_ASSERT_TRUE(db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::PinActive);
	EMB_ASSERT_TRUE(!db1.stateChanged());


	out.set(emb::PinInactive);
	EMB_ASSERT_EQUAL(in1.read(), emb::PinInactive);
	EMB_ASSERT_EQUAL(db1.state(), emb::PinActive);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::PinActive);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::PinActive);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::PinInactive);
	EMB_ASSERT_TRUE(db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::PinInactive);
	EMB_ASSERT_TRUE(!db1.stateChanged());


	out.set(emb::PinInactive);
	mcu::gpio::Input in2(in2Cfg);
	mcu::gpio::InputDebouncer db2(in2, 10, 40, 20);
	EMB_ASSERT_EQUAL(in2.read(), emb::PinActive);
	EMB_ASSERT_EQUAL(db2.state(), emb::PinInactive);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PinInactive);
	db2.debounce();
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PinInactive);
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PinActive);
	EMB_ASSERT_TRUE(db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PinActive);
	EMB_ASSERT_TRUE(!db2.stateChanged());

	out.set(emb::PinActive);
	EMB_ASSERT_EQUAL(in2.read(), emb::PinInactive);
	EMB_ASSERT_EQUAL(db2.state(), emb::PinActive);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PinActive);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PinInactive);
	EMB_ASSERT_TRUE(db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PinInactive);
	EMB_ASSERT_TRUE(!db2.stateChanged());

	out.set(emb::PinInactive);
	EMB_ASSERT_EQUAL(in2.read(), emb::PinActive);
	EMB_ASSERT_EQUAL(db2.state(), emb::PinInactive);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PinInactive);
	db2.debounce();
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PinInactive);
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PinActive);
	EMB_ASSERT_TRUE(db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::PinActive);
	EMB_ASSERT_TRUE(!db2.stateChanged());
#elif defined(TEST_BUILD)
#warning "LAUNCHXL is required for full testing."
#endif
}

///
///
///
void TestingDelayedTask()
{
	mcu::turnLedOn(mcu::LED_RED);
}

///
///
///
void McuTest::ClockTest()
{
	mcu::turnLedOff(mcu::LED_RED);

	mcu::SystemClock::registerDelayedTask(TestingDelayedTask, 200);
	DEVICE_DELAY_US(150000);
	mcu::SystemClock::runTasks();
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 1);
	DEVICE_DELAY_US(100000);
	mcu::SystemClock::runTasks();
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 0);

	mcu::turnLedOff(mcu::LED_RED);
}









