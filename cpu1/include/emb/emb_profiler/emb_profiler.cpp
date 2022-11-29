/**
 * @file emb_profiler.cpp
 * @ingroup emb
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "emb_profiler.h"


namespace emb {
/// @addtogroup emb
/// @{


uint64_t timeNowFuncNone_us()
{
	return 0;
}
uint64_t (*DurationLogger_us::s_timeNowFunc)() = timeNowFuncNone_us;


uint32_t timeNowFuncNone_clk()
{
	return 0;
}
uint32_t (*DurationLogger_clk::s_timeNowFunc)() = timeNowFuncNone_clk;


uint64_t (*DurationLoggerAsync_us::s_timeNowFunc)() = timeNowFuncNone_us;
DurationLoggerAsync_us::DurationData DurationLoggerAsync_us::s_durations_us[s_capacity];


/// @}
}


