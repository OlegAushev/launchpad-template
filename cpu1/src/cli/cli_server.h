/**
 * @file cli_server.h
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


#include <cstring>
#include <cstdio>
#include "emb/emb_interfaces/emb_uart.h"
#include "emb/emb_interfaces/emb_gpio.h"
#include "emb/emb_queue.h"
#include "emb/emb_string.h"
#include "emb/emb_circularbuffer.h"

#include "cli_config.h"


namespace cli {
/// @addtogroup cli
/// @{


/**
 * @brief CLI server.
 */
class Server
{
	friend void print(const char* str);
	friend void print_blocking(const char* str);
private:
	static emb::IUart* _uart;
	static emb::gpio::IOutput* _pinRTS;
	static emb::gpio::IInput* _pinCTS;

	static char _prompt[CLI_PROMPT_MAX_LENGTH];
	static emb::String<CLI_CMDLINE_MAX_LENGTH> _cmdline;
	static emb::String<CLI_ESCSEQ_MAX_LENGTH> _escseq;

	static size_t _cursorPos;

	static emb::Queue<char, CLI_OUTBUT_BUFFER_LENGTH> _outputBuf;

#ifdef CLI_USE_HISTORY
	static emb::CircularBuffer<emb::String<CLI_CMDLINE_MAX_LENGTH>, CLI_HISTORY_LENGTH> _history;
	static size_t _lastCmdHistoryPos;
	static size_t _historyPosition;
	static bool _newCmdSaved;
#endif

private:
	Server(const Server& other);		// no copy constructor
	Server& operator=(const Server& other);	// no copy assignment operator
public:
	Server(const char* deviceName, emb::IUart* uart, emb::gpio::IOutput* pinRTS, emb::gpio::IInput* pinCTS);

	void run();
	void registerExecCallback(int (*exec_)(int argc, const char** argv))
	{
		_exec = exec_;
	}

private:
	static void _print(char ch);
	static void _print(const char* str);
	static void _printBlocking(const char* str);

	static void _processChar(char ch);
	static void _saveCursorPos() { _print(CLI_ESC"[s"); }
	static void _loadCursorPos() { _print(CLI_ESC"[u"); }
	static void _moveCursor(int offset);
	static void _printWelcome();
	static void _printPrompt();
	static int _tokenize(const char** argv, emb::String<CLI_CMDLINE_MAX_LENGTH>& cmdline);

	static int (*_exec)(int argc, const char** argv);
	static int _execNull(int argc, const char** argv)
	{
		_print(CLI_ENDL"error: exec-callback not registered");
		_print(CLI_ENDL"tokens:");
		for (size_t i= 0; i < argc; ++i)
		{
			_print(CLI_ENDL);
			_print(argv[i]);
		}
		return -1;
	}

public:
	/**
	 * @brief
	 */
	struct EscSeq
	{
		const char* str;
		size_t len;
		void (*handler)();
	};
	static const EscSeq escSeqList[];
private:
	static void _escReturn();
	static void _escMoveCursorLeft();
	static void _escMoveCursorRight();
	static void _escHome();
	static void _escEnd();
	static void _escBack();
	static void _escDel();
	static void _escUp();
	static void _escDown();

private:
#ifdef CLI_USE_HISTORY
	enum HistorySearchDirection
	{
		CLI_HISTORY_SEARCH_UP,
		CLI_HISTORY_SEARCH_DOWN,
	};
	static void _searchHistory(HistorySearchDirection dir);
#endif
};


inline void print(const char* str)
{
	Server::_print(str);
}


inline void nextline()
{
	print(CLI_ENDL);
}


inline void print_blocking(const char* str)
{
	Server::_printBlocking(str);
}


inline void nextline_blocking()
{
	print_blocking(CLI_ENDL);
}


/// @}
} // namespace cli


