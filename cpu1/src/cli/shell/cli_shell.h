/**
 * @file cli_shell.h
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


#include "cli/cli_server.h"
#include <algorithm>
#include "emb/emb_algorithm.h"


#define CLI_CMD_OUTPUT_LENGTH CLI_OUTBUT_BUFFER_LENGTH
extern char CLI_CMD_OUTPUT[CLI_CMD_OUTPUT_LENGTH];


namespace cli {
/// @addtogroup cli
/// @{


/**
 * @brief Shell command specification.
 */
struct Cmd
{
	const char* name;
	int (*exec)(int argc, const char** argv);
	const char* help;
};


inline bool operator<(const Cmd& lhs, const Cmd& rhs)
{
	return strcmp(lhs.name, rhs.name) < 0;
}


inline bool operator<(const char* name, const Cmd& cmd)
{
	return strcmp(name, cmd.name) < 0;
}


inline bool operator==(const char* name, const Cmd& cmd)
{
	return strcmp(name, cmd.name) == 0;
}



/**
 * @brief CLI Shell.
 */
class Shell
{
private:
	static Cmd _commands[];
	static const size_t _commandsCount;
	static Cmd* _commandsEnd;
public:
	static void init();
	static int exec(int argc, const char** argv);
private:
	static int _list(int argc, const char** argv);
};





/// @}
} // namespace cli


