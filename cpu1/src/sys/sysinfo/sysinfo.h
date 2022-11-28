/**
 * @defgroup sysinfo SysInfo
 *
 * @file
 * @ingroup sysinfo
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


