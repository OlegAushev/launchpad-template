/**
 * @file cli_uptime.cpp
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


int cli_uptime(int argc, const char** argv)
{
	if (argc == 0)
	{
		snprintf(CLI_CMD_OUTPUT, CLI_CMD_OUTPUT_LENGTH, "uptime: %llums", mcu::chrono::SystemClock::now());
	}
	else
	{
		if (strcmp(argv[0], "-v") == 0)
		{
			uint64_t sec = mcu::chrono::SystemClock::now() / 1000;
			uint64_t msec = mcu::chrono::SystemClock::now() - 1000 * sec;
			uint64_t min = sec / 60;
			sec -= 60 * min;

			float uptime = mcu::chrono::SystemClock::now() / 1000.f;
			snprintf(CLI_CMD_OUTPUT, CLI_CMD_OUTPUT_LENGTH, "uptime: %llum %llus %llums", min, sec, msec);
		}
		else
		{
			snprintf(CLI_CMD_OUTPUT, CLI_CMD_OUTPUT_LENGTH, "uptime: invalid option - \"%s\"", argv[0]);
		}
	}

	cli::nextline();
	cli::print(CLI_CMD_OUTPUT);
	return 0;
}


