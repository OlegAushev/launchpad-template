///
///
///
#pragma once


#include "emb/emb_testrunner/emb_testrunner.h"
#include "mcu_f2837xd/system/mcu_system.h"
#include "mcu_f2837xd/gpio/mcu_gpio.h"
#include "mcu_f2837xd/chrono/mcu_chrono.h"
#include "mcu_f2837xd/support/mcu_support.h"


class McuTest
{
public:
	static void GpioTest();
	static void ChronoTest();
};


