/**
 * @file syslog_conf.h
 * @ingroup syslog
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once


#include "emb/emb_core.h"


namespace sys {
/// @addtogroup syslog
/// @{


/// System errors
SCOPED_ENUM_UT_DECLARE_BEGIN(Error, uint32_t)
{

}
SCOPED_ENUM_DECLARE_END(Error)


/// Fatal system errors
const uint32_t fatalErrors = 0;


/// System warnings
SCOPED_ENUM_UT_DECLARE_BEGIN(Warning, uint32_t)
{
	CanBusError,
	CanBusOverrun,
}
SCOPED_ENUM_DECLARE_END(Warning)


/// Fatal system warnings
const uint32_t fatalWarnings = 0;


/// System messages
SCOPED_ENUM_UT_DECLARE_BEGIN(Message, uint32_t)
{
	NoMessage,
	DeviceCpu1Booting,
	DeviceCpu1BootSuccess,
	DeviceCpu2Booting,
	DeviceCpu2BootSuccess,
	DeviceCpu1Ready,
	DeviceCpu2Ready,
	DeviceReady,
	DeviceBusy,
	DeviceSoftwareResetting,
	CanSdoRequestLost,
}
SCOPED_ENUM_DECLARE_END(Message)


/// @}
} // namespace sys


