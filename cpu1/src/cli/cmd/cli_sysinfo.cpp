/**
 * @file cli_sysinfo.cpp
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

#include "sys/sysinfo/sysinfo.h"


int cli_sysinfo(int argc, const char** argv)
{
	cli::nextline();
	cli::print("device name: ");
	cli::print(SysInfo::deviceName);
	cli::nextline();
	cli::print("firmware version: ");
	cli::print(SysInfo::firmwareVersion);
	cli::nextline();
	cli::print("build configuration: ");
	cli::print(SysInfo::buildConfiguration);
	return 0;
}


