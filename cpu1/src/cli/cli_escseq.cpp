/**
 * @file cli_escseq.cpp
 * @ingroup cli
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "cli_server.h"


namespace cli {


///
///
///
void Server::_escReturn()
{
#ifdef CLI_USE_HISTORY
	if (!_cmdline.empty())
	{
		if (_history.empty())
		{
			_history.push(_cmdline);
			_lastCmdHistoryPos = 0;
		}
		else if (_cmdline != _history.back())
		{
			_history.push(_cmdline);
			_lastCmdHistoryPos = (_lastCmdHistoryPos + 1) % _history.capacity();
		}
	}
	_newCmdSaved = true;
	_historyPosition = _lastCmdHistoryPos;
#endif

	const char* argv[CLI_TOKEN_MAX_COUNT];
	int argc = _tokenize(argv, _cmdline);

	switch (argc)
	{
	case -1:
		print(CLI_ENDL);
		print("error: too many tokens");
		break;
	case 0:
		break;
	default:
		_exec(argc, argv);
		break;
	}

	_printPrompt();
}

///
///
///
void Server::_escMoveCursorLeft()
{
	if (_cursorPos > 0)
	{
		--_cursorPos;
		print(CLI_ESC"[D");
	}

}


///
///
///
void Server::_escMoveCursorRight()
{
	if (_cursorPos < _cmdline.lenght())
	{
		++_cursorPos;
		print(CLI_ESC"[C");
	}
}


///
///
///
void Server::_escHome()
{
	if (_cursorPos > 0)
	{
		_moveCursor(-_cursorPos);
		_cursorPos = 0;
	}
}


///
///
///
void Server::_escEnd()
{
	if (_cursorPos < _cmdline.lenght())
	{
		_moveCursor(_cmdline.lenght() - _cursorPos);
		_cursorPos = _cmdline.lenght();
	}
}


///
///
///
void Server::_escBack()
{
	if (_cursorPos > 0)
	{
		memmove(_cmdline.begin() + _cursorPos - 1,
				_cmdline.begin() + _cursorPos,
				_cmdline.lenght() - _cursorPos);
		_cmdline.pop_back();
		--_cursorPos;

		print(CLI_ESC"[D"" "CLI_ESC"[D");	// delete symbol
		_saveCursorPos();
		print(_cmdline.begin() + _cursorPos);
		print(" ");				// hide last symbol
		_loadCursorPos();
	}
}


///
///
///
void Server::_escDel()
{
	if (_cursorPos < _cmdline.lenght())
	{
		memmove(_cmdline.begin() + _cursorPos,
				_cmdline.begin() + _cursorPos + 1,
				_cmdline.lenght() - _cursorPos);
		_cmdline.pop_back();

		_saveCursorPos();
		print(_cmdline.begin() + _cursorPos);
		print(" ");
		_loadCursorPos();
	}
}


///
///
///
void Server::_escUp()
{
#ifdef CLI_USE_HISTORY
	if (!_history.empty())
	{
		_searchHistory(CLI_HISTORY_SEARCH_UP);
	}
#endif
}


///
///
///
void Server::_escDown()
{
#ifdef CLI_USE_HISTORY
	if (!_history.empty())
	{
		_searchHistory(CLI_HISTORY_SEARCH_DOWN);
	}
#endif
}


} // namespace cli


