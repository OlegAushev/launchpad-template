/**
 * @file cli_sysctl.cpp
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


//#include ""


int cli_sysctl(int argc, const char** argv)
{
	if (argc == 0)
	{
		strncpy(CLI_CMD_OUTPUT, "Options not specified.", CLI_CMD_OUTPUT_LENGTH);
		goto cli_syslog_print;
	}

	if (strcmp(argv[0], "startup") == 0)
	{
		//fuelcell::Converter::instance()->startup();
		//strncpy(CLI_CMD_OUTPUT, "Charger startup...", CLI_CMD_OUTPUT_LENGTH);
		goto cli_syslog_print;
	}

	if (strcmp(argv[0], "shutdown") == 0)
	{
		//fuelcell::Converter::instance()->shutdown();
		//strncpy(CLI_CMD_OUTPUT, "Charger shutdown...", CLI_CMD_OUTPUT_LENGTH);
		goto cli_syslog_print;
	}

	strncpy(CLI_CMD_OUTPUT, "Invalid options.", CLI_CMD_OUTPUT_LENGTH);

cli_syslog_print:
	cli::nextline();
	cli::print(CLI_CMD_OUTPUT);
	return 0;

}


