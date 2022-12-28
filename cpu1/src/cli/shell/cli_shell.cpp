/**
 * @file cli_shell.cpp
 * @ingroup cli
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "cli_shell.h"

#include "mcu_f2837xd/system/mcu_system.h"
#include "mcu_f2837xd/chrono/mcu_chrono.h"


int cli_sysinfo(int argc, const char** argv);
int cli_reboot(int argc, const char** argv);
int cli_uptime(int argc, const char** argv);
int cli_syslog(int argc, const char** argv);
int cli_sysctl(int argc, const char** argv);


extern char CLI_CMD_OUTPUT[CLI_CMD_OUTPUT_LENGTH] = {0};


namespace cli {


Cmd Shell::_commands[] =
{
{"list",		Shell::_list, 		"Prints all available commands."},
{"sysinfo",		cli_sysinfo,		"Prints basic information about system."},
{"reboot",		cli_reboot,		"Reboots device."},
{"uptime",		cli_uptime,		"Shows system uptime."},
{"syslog",		cli_syslog,		"SysLog control utility."},
{"sysctl",		cli_sysctl,		"System control utility."},
};

const size_t Shell::_commandsCount = sizeof(Shell::_commands) / sizeof(Shell::_commands[0]);
Cmd* Shell::_commandsEnd = Shell::_commands + Shell::_commandsCount;


///
///
///
void Shell::init()
{
	std::sort(_commands, _commandsEnd);
}


///
///
///
int Shell::exec(int argc, const char** argv)
{
	if (argc == 0) return 0;

	const Cmd* cmd = emb::binary_find(_commands, _commandsEnd, argv[0]);
	if (cmd == _commandsEnd)
	{
		cli::nextline();
		cli::print(argv[0]);
		cli::print(": command not found");
		return -1;
	}

	if (argc == 1)
	{
		return cmd->exec(--argc, ++argv);
	}
	else if (strcmp(argv[1], "--help") == 0)
	{
		cli::nextline();
		cli::print(cmd->help);
		return 0;
	}
	else
	{
		return cmd->exec(--argc, ++argv);
	}
}


///
///
///
int Shell::_list(int argc, const char** argv)
{
	cli::nextline();
	cli::print("Available commands are:");
	for (size_t i = 0; i < _commandsCount; ++i)
	{
		cli::nextline();
		cli::print(_commands[i].name);
	}
	return 0;
}















} // namespace cli


