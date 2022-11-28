/**
 * @file
 * @ingroup cli
 */


#pragma once


#include "cli/shell/cli_shell.h"

#include "sys/sysinfo/sysinfo.h"


int cli_sysinfo(int argc, const char** argv)
{
	cli::nextline();
	cli::print("device name: ");
	cli::print(sys::Sysinfo::deviceName);
	cli::nextline();
	cli::print("firmware version: ");
	cli::print(sys::Sysinfo::firmwareVersion);
	cli::nextline();
	cli::print("build configuration: ");
	cli::print(sys::Sysinfo::buildConfiguration);
	return 0;
}


