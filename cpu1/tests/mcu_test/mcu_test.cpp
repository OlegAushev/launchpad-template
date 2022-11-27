///
#include "mcu_test.h"

///
///
///
void McuTest::GpioTest()
{
#ifdef _LAUNCHXL_F28379D
	mcu::gpio::Config ledBlueCfg(31, GPIO_31_GPIO31, mcu::gpio::Direction::Output, emb::gpio::ActiveState::High, mcu::gpio::Type::Std, mcu::gpio::QualMode::Sync, 1);
	mcu::gpio::Config ledRedCfg(34, GPIO_34_GPIO34, mcu::gpio::Direction::Output, emb::gpio::ActiveState::Low, mcu::gpio::Type::Std, mcu::gpio::QualMode::Sync, 1);

	mcu::gpio::Output ledBlue(ledBlueCfg);
	mcu::gpio::Output ledRed(ledRedCfg);

	GPIO_writePin(31, 1);
	GPIO_writePin(34, 1);

	EMB_ASSERT_EQUAL(ledBlue.read(), emb::gpio::State::Active);
	EMB_ASSERT_EQUAL(ledRed.read(), emb::gpio::State::Inactive);

	ledBlue.set(emb::gpio::State::Inactive);	// led - on
	ledRed.set(emb::gpio::State::Inactive);		// led - off
	DEVICE_DELAY_US(100000);

	EMB_ASSERT_EQUAL(GPIO_readPin(31), 0);
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 1);
	EMB_ASSERT_EQUAL(ledBlue.read(), emb::gpio::State::Inactive);
	EMB_ASSERT_EQUAL(ledRed.read(), emb::gpio::State::Inactive);

	ledBlue.set(emb::gpio::State::Active);		// led - off
	ledRed.set(emb::gpio::State::Active);		// led - on
	DEVICE_DELAY_US(100000);

	EMB_ASSERT_EQUAL(GPIO_readPin(31), 1);
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 0);
	EMB_ASSERT_EQUAL(ledBlue.read(), emb::gpio::State::Active);
	EMB_ASSERT_EQUAL(ledRed.read(), emb::gpio::State::Active);

	GPIO_writePin(31, 1);
	GPIO_writePin(34, 1);
#elif defined(TEST_BUILD)
#warning "LAUNCHXL is required for full testing."
#endif

#ifdef _LAUNCHXL_F28379D
	mcu::gpio::Config outCfg(27, GPIO_27_GPIO27, mcu::gpio::Direction::Output, emb::gpio::ActiveState::High, mcu::gpio::Type::Std, mcu::gpio::QualMode::Sync, 1);
	mcu::gpio::Config in1Cfg(25, GPIO_25_GPIO25, mcu::gpio::Direction::Input, emb::gpio::ActiveState::High, mcu::gpio::Type::Std, mcu::gpio::QualMode::Sync, 1);
	mcu::gpio::Config in2Cfg(25, GPIO_25_GPIO25, mcu::gpio::Direction::Input, emb::gpio::ActiveState::Low, mcu::gpio::Type::Std, mcu::gpio::QualMode::Sync, 1);

	mcu::gpio::Output out(outCfg);
	mcu::gpio::Input in1(in1Cfg);
	mcu::gpio::InputDebouncer db1(in1, 10, 20, 30);

	EMB_ASSERT_EQUAL(in1.read(), emb::gpio::State::Inactive);
	EMB_ASSERT_EQUAL(db1.state(), emb::gpio::State::Inactive);
	EMB_ASSERT_TRUE(!db1.stateChanged());

	out.set(emb::gpio::State::Active);
	EMB_ASSERT_EQUAL(in1.read(), emb::gpio::State::Active);
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::gpio::State::Inactive);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::gpio::State::Active);
	EMB_ASSERT_TRUE(db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::gpio::State::Active);
	EMB_ASSERT_TRUE(!db1.stateChanged());


	out.set(emb::gpio::State::Inactive);
	EMB_ASSERT_EQUAL(in1.read(), emb::gpio::State::Inactive);
	EMB_ASSERT_EQUAL(db1.state(), emb::gpio::State::Active);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::gpio::State::Active);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::gpio::State::Active);
	EMB_ASSERT_TRUE(!db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::gpio::State::Inactive);
	EMB_ASSERT_TRUE(db1.stateChanged());
	db1.debounce();
	EMB_ASSERT_EQUAL(db1.state(), emb::gpio::State::Inactive);
	EMB_ASSERT_TRUE(!db1.stateChanged());


	out.set(emb::gpio::State::Inactive);
	mcu::gpio::Input in2(in2Cfg);
	mcu::gpio::InputDebouncer db2(in2, 10, 40, 20);
	EMB_ASSERT_EQUAL(in2.read(), emb::gpio::State::Active);
	EMB_ASSERT_EQUAL(db2.state(), emb::gpio::State::Inactive);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::gpio::State::Inactive);
	db2.debounce();
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::gpio::State::Inactive);
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::gpio::State::Active);
	EMB_ASSERT_TRUE(db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::gpio::State::Active);
	EMB_ASSERT_TRUE(!db2.stateChanged());

	out.set(emb::gpio::State::Active);
	EMB_ASSERT_EQUAL(in2.read(), emb::gpio::State::Inactive);
	EMB_ASSERT_EQUAL(db2.state(), emb::gpio::State::Active);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::gpio::State::Active);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::gpio::State::Inactive);
	EMB_ASSERT_TRUE(db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::gpio::State::Inactive);
	EMB_ASSERT_TRUE(!db2.stateChanged());

	out.set(emb::gpio::State::Inactive);
	EMB_ASSERT_EQUAL(in2.read(), emb::gpio::State::Active);
	EMB_ASSERT_EQUAL(db2.state(), emb::gpio::State::Inactive);
	EMB_ASSERT_TRUE(!db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::gpio::State::Inactive);
	db2.debounce();
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::gpio::State::Inactive);
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::gpio::State::Active);
	EMB_ASSERT_TRUE(db2.stateChanged());
	db2.debounce();
	EMB_ASSERT_EQUAL(db2.state(), emb::gpio::State::Active);
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
	GPIO_writePin(34, 0);
}

///
///
///
void McuTest::ChronoTest()
{
	GPIO_writePin(34, 1);

	mcu::chrono::SystemClock::registerDelayedTask(TestingDelayedTask, 200);
	DEVICE_DELAY_US(150000);
	mcu::chrono::SystemClock::runTasks();
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 1);
	DEVICE_DELAY_US(100000);
	mcu::chrono::SystemClock::runTasks();
	EMB_ASSERT_EQUAL(GPIO_readPin(34), 0);

	GPIO_writePin(34, 1);
}









