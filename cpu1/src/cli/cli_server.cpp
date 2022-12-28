/**
 * @file cli_server.cpp
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


const char* promptBegin = CLI_PROMPT_BEGIN;
const char* promptEnd = CLI_PROMPT_END;


emb::IUart* Server::_uart = static_cast<emb::IUart*>(NULL);
emb::gpio::IOutput* Server::_pinRTS = static_cast<emb::gpio::IOutput*>(NULL);
emb::gpio::IInput* Server::_pinCTS = static_cast<emb::gpio::IInput*>(NULL);

char Server::_prompt[CLI_PROMPT_MAX_LENGTH] = {0};
emb::String<CLI_CMDLINE_MAX_LENGTH> Server::_cmdline;
emb::String<CLI_ESCSEQ_MAX_LENGTH> Server::_escseq;

size_t Server::_cursorPos = 0;

emb::Queue<char, CLI_OUTBUT_BUFFER_LENGTH> Server::_outputBuf;

#ifdef CLI_USE_HISTORY
emb::CircularBuffer<emb::String<CLI_CMDLINE_MAX_LENGTH>, CLI_HISTORY_LENGTH> Server::_history;
size_t Server::_lastCmdHistoryPos = 0;
size_t Server::_historyPosition = 0;
bool Server::_newCmdSaved = false;
#endif

int (*Server::_exec)(int argc, const char** argv) = Server::_execNull;


const Server::EscSeq Server::escSeqList[] = {
{.str = "\x0D",		.len = 1,	.handler = Server::_escReturn},
{.str = "\x0A",		.len = 1,	.handler = Server::_escReturn},
{.str = CLI_ESC"[D",	.len = 3,	.handler = Server::_escMoveCursorLeft},
{.str = CLI_ESC"[C",	.len = 3,	.handler = Server::_escMoveCursorRight},
{.str = CLI_ESC"[H",	.len = 3,	.handler = Server::_escHome},
{.str = CLI_ESC"[F",	.len = 3,	.handler = Server::_escEnd},
{.str = "\x08",		.len = 1,	.handler = Server::_escBack},
{.str = "\x7F",		.len = 1,	.handler = Server::_escBack},
{.str = CLI_ESC"[3~",	.len = 4,	.handler = Server::_escDel},
{.str = CLI_ESC"[A",	.len = 3,	.handler = Server::_escUp},
{.str = CLI_ESC"[B",	.len = 3,	.handler = Server::_escDown},
};

const size_t ESCSEQ_LIST_SIZE = sizeof(Server::escSeqList) / sizeof(Server::escSeqList[0]);


///
///
///
Server::Server(const char* deviceName, emb::IUart* uart, emb::gpio::IOutput* pinRTS, emb::gpio::IInput* pinCTS)
{
	_uart = uart;
	_pinRTS = pinRTS;	// output
	_pinCTS = pinCTS;	// input

	memset(_prompt, 0, CLI_PROMPT_MAX_LENGTH);
	strcat(_prompt, promptBegin);
	strncat(_prompt, deviceName, CLI_DEVICE_NAME_MAX_LENGTH);
	strcat(_prompt, promptEnd);

	_printPrompt();
}


///
///
///
void Server::run()
{
	if (!_outputBuf.empty())
	{
		if (_uart->send(_outputBuf.front()))
		{
			_outputBuf.pop();
		}
	}
	else
	{
		char ch;
		if (_uart->recv(ch))
		{
			_processChar(ch);
		}
	}
}


///
///
///
void Server::_print(char ch)
{
	if (!_outputBuf.full())
	{
		_outputBuf.push(ch);
	}
}


///
///
///
void Server::_print(const char* str)
{
	while ((*str != '\0') && !_outputBuf.full())
	{
		_outputBuf.push(*str++);
	}
}


///
///
///
void Server::_printBlocking(const char* str)
{
	_uart->send(str, strlen(str));
}


///
///
///
void Server::_processChar(char ch)
{
	if (_cmdline.full())
		return;

	if (_escseq.empty())
	{
		// Check escape signature
		if (ch <= 0x1F || ch == 0x7F)
		{
			_escseq.push_back(ch);
		}
		// Print symbol if escape sequence signature is not found
		if (_escseq.empty())
		{
			if (_cursorPos < _cmdline.lenght())
			{
				_cmdline.insert(_cursorPos, ch);
				_saveCursorPos();
				print(_cmdline.begin() + _cursorPos);
				_loadCursorPos();
			}
			else
			{
				_cmdline.push_back(ch);
			}
			_print(ch);
			++_cursorPos;
		}
	}
	else
	{
		_escseq.push_back(ch);
	}

	// Process escape sequence
	if (!_escseq.empty())
	{
		int possibleEscseqCount = 0;
		size_t escseqIdx = 0;
		for (size_t i = 0; i < ESCSEQ_LIST_SIZE; ++i)
		{
			if ((_escseq.lenght() <= escSeqList[i].len)
					&& (strncmp(_escseq.data(), escSeqList[i].str, _escseq.lenght()) == 0))
			{
				++possibleEscseqCount;
				escseqIdx = i;
			}
		}

		switch (possibleEscseqCount)
		{
		case 0: // no sequence - display all symbols
			/*for (size_t i = 0; (i < m_escseq.lenght()) && (!m_cmdline.full()); ++i)
			{
				if (m_escseq[i] <= 0x1F || m_escseq[i] == 0x7F)
				{
					m_escseq[i] = '?';
				}
				m_cmdline.insert(m_cursorPos + i, m_escseq[i]);
			}
			print(m_cmdline.begin() + m_cursorPos);
			m_cursorPos += m_escseq.lenght();*/
			_escseq.clear();
			break;

		case 1: // one possible sequence found - check size and call handler
			if (_escseq.lenght() == escSeqList[escseqIdx].len)
			{
				_escseq.clear();
				escSeqList[escseqIdx].handler();
			}
			break;

		default: // few possible sequences found
			break;
		}
	}
}


///
///
///
void Server::_moveCursor(int offset)
{
	char str[16] = {0};
	if (offset > 0) {
		snprintf(str, 16, CLI_ESC"[%dC", offset);
	} else if (offset < 0) {
		snprintf(str, 16, CLI_ESC"[%dD", -(offset));
	}
	print(str);
}


///
///
///
void Server::_printPrompt()
{
	print(CLI_ENDL);
	print(_prompt);
	_cmdline.clear();
	_cursorPos = 0;
}


///
///
///
int Server::_tokenize(const char** argv, emb::String<CLI_CMDLINE_MAX_LENGTH>& cmdline)
{
	int argc = 0;
	size_t idx = 0;

	if (cmdline.empty())
	{
		return 0;
	}

	// replace all ' ' with '\0'
	for (size_t i = 0; i < cmdline.lenght(); ++i)
	{
		if (cmdline[i] == ' ')
		{
			cmdline[i] = '\0';
		}
	}

	while (true)
	{
		// go to the first not-whitespace (now - '\0')
		while (cmdline[idx] == '\0')
		{
			if (++idx >= cmdline.lenght())
				return argc;
		}

		if (argc >= CLI_TOKEN_MAX_COUNT)
			return -1;

		argv[argc++] = cmdline.begin() + idx;

		// go to the first whitespace (now - '\0')
		while ((cmdline[idx] != '\0') && (idx < cmdline.lenght()))
		{
			if (++idx >= cmdline.lenght())
				return argc;
		}
	}
}


#ifdef CLI_USE_HISTORY
///
///
///
void Server::_searchHistory(HistorySearchDirection dir)
{
	static size_t pos;

	switch (dir)
	{
	case CLI_HISTORY_SEARCH_UP:
		if (_newCmdSaved)
		{
			pos = _historyPosition;
		}
		else
		{
			_historyPosition = (_historyPosition + (_history.size() - 1)) % _history.size();
			pos = _historyPosition;
		}
		break;
	case CLI_HISTORY_SEARCH_DOWN:
		_historyPosition = (_historyPosition + 1) % _history.size();
		pos = _historyPosition;
		break;
	}

	_newCmdSaved = false;

	// move cursor to line beginning
	if (_cursorPos > 0)
	{
		_moveCursor(-_cursorPos);
		_cursorPos = 0;
	}

	int remainder = _cmdline.size() - _history.data()[pos].size();
	_cmdline = _history.data()[pos];
	print(_cmdline.data());
	_cursorPos = _cmdline.size();

	// clear remaining symbols
	_saveCursorPos();
	for (int i = 0; i < remainder; ++i)
	{
		print(" ");
	}
	_loadCursorPos();
}
#endif


} // namespace cli


