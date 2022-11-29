///


#include "mcu_f2837xd/adc/mcu_adc.h"
#include "emb/emb_testrunner/emb_testrunner.h"
#include "emb/tests/emb_test.h"

#include "sys/syslog/syslog.h"
#include "mcu_test/mcu_test.h"


void emb::run_tests()
{
	SysLog::init(SysLog::IpcFlags());
	mcu::chrono::SystemClock::init();

	mcu::adc::Configuration adcConf =
	{
		.sampleWindow_ns = 200,
	};
	mcu::adc::Module adc(adcConf);
	adc.enableInterrupts();

#ifdef _LAUNCHXL_F28379D
	for (size_t i = 0; i < 10; ++i)
	{
		bsp::ledBlue.toggle();
		mcu::delay_us(50000);
		bsp::ledRed.toggle();
		mcu::delay_us(50000);
	}
#endif
	mcu::delay_us(100000);
	mcu::enableMaskableInterrupts();
	mcu::enableDebugEvents();

	EMB_RUN_TEST(EmbTest::CommonTest);
	EMB_RUN_TEST(EmbTest::MathTest);
	EMB_RUN_TEST(EmbTest::AlgorithmTest);
	EMB_RUN_TEST(EmbTest::ArrayTest);
	EMB_RUN_TEST(EmbTest::QueueTest);
	EMB_RUN_TEST(EmbTest::CircularBufferTest);
	EMB_RUN_TEST(EmbTest::FilterTest);
	EMB_RUN_TEST(EmbTest::StackTest);
	EMB_RUN_TEST(EmbTest::BitsetTest);
	EMB_RUN_TEST(EmbTest::StaticVectorTest);
	EMB_RUN_TEST(EmbTest::StringTest);

	EMB_RUN_TEST(McuTest::GpioTest);
	EMB_RUN_TEST(McuTest::ChronoTest);

	emb::TestRunner::printResult();

	while (true)
	{
#ifdef _LAUNCHXL_F28379D
		if (emb::TestRunner::passed())
		{
			bsp::ledBlue.toggle();
		}
		else
		{
			bsp::ledRed.toggle();
		}
#endif
		mcu::delay_us(500000);
	}
}




