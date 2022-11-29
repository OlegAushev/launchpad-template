/**
 * @defgroup sysinfo SysInfo
 *
 * @file sysinfo.h
 * @ingroup sysinfo
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


/// @addtogroup sysinfo
/// @{


class SysInfo
{
public:
	static const char* deviceName;
	static const char* deviceNameShort;

	static const char* firmwareVersion;
	static const uint32_t firmwareVersionNum;

	static const char* buildConfiguration;
	static const char* buildConfigurationShort;
};


/// @}


