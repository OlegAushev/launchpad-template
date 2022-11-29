/**
 * @file cli_reboot.cpp
 * @ingroup cli
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once


#include "cli/shell/cli_shell.h"

#include "mcu_f2837xd/chrono/mcu_chrono.h"
#include "mcu_f2837xd/system/mcu_system.h"


int cli_reboot(int argc, const char** argv)
{
	cli::nextline();

	if (argc > 1)
	{
		cli::print("Invalid options.");
		return 0;
	}

	if (argc == 0)
	{
		mcu::chrono::SystemClock::registerDelayedTask(mcu::resetDevice, 2000);
		cli::print("Device will reboot in 2 seconds...");
		return 0;
	}

	uint64_t msec = 1000 * atoll(argv[0]);
	mcu::chrono::SystemClock::registerDelayedTask(mcu::resetDevice, msec);
	snprintf(CLI_CMD_OUTPUT, CLI_CMD_OUTPUT_LENGTH, "Device will reboot in %llu seconds...", msec / 1000);
	cli::print(CLI_CMD_OUTPUT);
	return 0;
}


