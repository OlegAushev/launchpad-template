/**
 * @file
 * @ingroup syslog
 */


#pragma once


namespace sys {
/// @addtogroup syslog
/// @{


namespace Error {


/// System errors
enum Error
{

};


const uint32_t FATAL_ERRORS = 0;


} // namespace Fault


namespace Warning {


/// System warnings
enum Warning
{

};


const uint32_t FATAL_WARNINGS = 0;


} // namespace Warning


namespace Message {


/// System messages
enum Message
{
	NO_MESSAGE,
	DEVICE_CPU1_BOOT,
	DEVICE_CPU1_BOOT_SUCCESS,
	DEVICE_CPU2_BOOT,
	DEVICE_CPU2_BOOT_SUCCESS,
	DEVICE_CPU1_READY,
	DEVICE_CPU2_READY,
	DEVICE_READY,
	DEVICE_BUSY,
	DEVICE_SW_RESET,
};


} // namespace Message


/// @}
} // namespace sys


