/**
 * @defgroup sysinfo Sysinfo
 *
 * @file
 * @ingroup sysinfo
 */


#pragma once


#include <stdint.h>
#include <stddef.h>


namespace sys {
/// @addtogroup sysinfo
/// @{


class Sysinfo
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
} // namespace sys


