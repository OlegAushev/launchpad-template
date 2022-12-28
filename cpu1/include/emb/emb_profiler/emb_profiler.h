/**
 * @file emb_profiler.h
 * @ingroup emb
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once


#include <stdint.h>
#include <stddef.h>
#include <cstdio>
#include <cstring>

#include "../emb_core.h"


namespace emb {
/// @addtogroup emb
/// @{


/**
 * @brief All calculations in ns
 */
class DurationLogger_us
{
private:
	static uint64_t (*_timeNowFunc)();
	static const size_t _messageLenMax = 32;
	char _message[_messageLenMax];
	volatile uint64_t _start;

public:
	explicit DurationLogger_us(const char* message)
	{
		strncpy(_message, message, _messageLenMax-1);
		_message[_messageLenMax-1] = '\0';
		_start = _timeNowFunc();
	}

	~DurationLogger_us()
	{
		volatile uint64_t finish = _timeNowFunc();
		if (finish < _start)
		{
			printf("%s: timer overflow\n", _message);
		}
		else
		{
			printf("%s: %.3f us\n", _message, float(finish - _start) / 1000.f);
		}
	}

	static void init(uint64_t (*timeNowFunc_ns)(void))
	{
		_timeNowFunc = timeNowFunc_ns;
	}
};


#define EMB_LOG_DURATION_us(message) \
		volatile emb::DurationLogger_us EMB_UNIQ_ID(__LINE__)(message);


/**
 * @brief All calculations in clock cycles
 */
class DurationLogger_clk
{
private:
	static uint32_t (*_timeNowFunc)();
	static const size_t _messageLenMax = 32;
	char _message[_messageLenMax];
	volatile uint32_t _start;

public:
	explicit DurationLogger_clk(const char* message)
	{
		strncpy(_message, message, _messageLenMax-1);
		_message[_messageLenMax-1] = '\0';
		_start = _timeNowFunc();
	}

	~DurationLogger_clk()
	{
		volatile uint32_t finish = _timeNowFunc();
		if (finish > _start)
		{
			printf("%s: timer overflow\n", _message);
		}
		else
		{
			printf("%s: %lu clock cycles\n", _message, _start - finish);
		}
	}

	static void init(uint32_t (*timeNowFunc_clk)(void))
	{
		_timeNowFunc = timeNowFunc_clk;
	}
};


#define EMB_LOG_DURATION_clk(message) \
		volatile emb::DurationLogger_clk EMB_UNIQ_ID(__LINE__)(message);


/**
 * @brief All calculations in ns
 */
class DurationLoggerAsync_us
{
private:
	static uint64_t (*_timeNowFunc)();
	static const size_t _capacity = 10;

	struct DurationData
	{
		const char* message;
		volatile float value;
		DurationData() : value(0) {}
	};

	static DurationData _durations_us[_capacity];

	const size_t _channel;
	volatile uint64_t _start;
public:
	DurationLoggerAsync_us(const char* message, size_t channel)
		: _channel(channel)
	{
		_durations_us[_channel].message = message;
		_start = _timeNowFunc();
	}

	~DurationLoggerAsync_us()
	{
		volatile uint64_t finish = _timeNowFunc();
		if (finish < _start)
		{
			_durations_us[_channel].value = 0;
		}
		else
		{
			_durations_us[_channel].value = float(finish - _start) / 1000.f;
		}
	}

	static void init(uint64_t (*timeNowFunc)(void))
	{
		_timeNowFunc = timeNowFunc;
	}

	static void print()
	{
		for (size_t i = 0; i < _capacity; ++i)
		{
			if (_durations_us[i].value != 0)
			{
				printf("%s: %.3f us\n", _durations_us[i].message, _durations_us[i].value);
			}
		}
	}
};


#define EMB_LOG_DURATION_ASYNC_us(message, channel) \
		volatile emb::DurationLoggerAsync_us EMB_UNIQ_ID(__LINE__)(message, channel);


/// @}
} // namespace emb


