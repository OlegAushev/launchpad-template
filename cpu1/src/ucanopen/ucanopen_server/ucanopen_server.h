/**
 * @file ucanopen_server.h
 * @ingroup ucanopen
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once


#include "../ucanopen_def.h"
#include "mcu_f2837xd/ipc/mcu_ipc.h"


namespace ucanopen {
/// @addtogroup ucanopen_server
/// @{


/**
 * @brief IPC flags.
 *
 */
struct IpcFlags
{
	mcu::ipc::Flag rpdo1Reveived;
	mcu::ipc::Flag rpdo2Reveived;
	mcu::ipc::Flag rpdo3Reveived;
	mcu::ipc::Flag rpdo4Reveived;
	mcu::ipc::Flag rsdoReveived;
};


/**
 * @brief uCANopen server interface.
 */
//template typename










/// @}
}


