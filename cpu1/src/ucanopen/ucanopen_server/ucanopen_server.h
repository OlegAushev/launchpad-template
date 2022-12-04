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
#include "mcu_f2837xd/can/mcu_can.h"


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
template <mcu::ipc::Mode::enum_type IpcMode, mcu::ipc::Role::enum_type IpcRole>
class IServer : public emb::c28x::interrupt_invoker<IServer<IpcMode, IpcRole> >
{
private:



public:
	IServer(NodeId nodeId, mcu::can::IModule* canModule)
		: emb::c28x::interrupt_invoker<IServer<IpcMode, IpcRole> >(this)
	{
		EMB_STATIC_ASSERT(IpcRole == mcu::ipc::Role::Primary);
	}

	IServer()
			: emb::c28x::interrupt_invoker<IServer<IpcMode, IpcRole> >(this)
	{
		EMB_STATIC_ASSERT(IpcMode != mcu::ipc::Mode::Singlecore);
		EMB_STATIC_ASSERT(IpcRole == mcu::ipc::Role::Secondary);
	}
};










/// @}
}


