/**
 * @file ucanopen_tests.h
 * @ingroup ucanopen
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief
 * @version 0.1
 * @date 2022-12-07
 *
 * @copyright Copyright (c) 2022
 *
 */


#pragma once


#include "../server/ucanopen_server.h"
#include "sys/sysinfo/sysinfo.h"
#include "auto-generated/git_version.h"


namespace ucanopen {
/// @addtogroup ucanopen_server
/// @{


namespace tests {


struct CobTpdo1
{
	uint64_t clock;
	CobTpdo1()
	{
		EMB_STATIC_ASSERT(sizeof(CobTpdo1) == 4);
		memset(this, 0, sizeof(CobTpdo1));
	}
};


struct CobTpdo2
{
	uint32_t milliseconds;
	uint32_t seconds;
	CobTpdo2()
	{
		EMB_STATIC_ASSERT(sizeof(CobTpdo2) == 4);
		memset(this, 0, sizeof(CobTpdo2));
	}
};


struct CobTpdo3
{
	uint32_t heartbeat : 8;
	uint32_t _reserved1 : 24;
	uint32_t _reserved2 : 32;
	CobTpdo3()
	{
		EMB_STATIC_ASSERT(sizeof(CobTpdo3) == 4);
		memset(this, 0, sizeof(CobTpdo3));
	}
};


struct CobTpdo4
{
	uint32_t errors : 32;
	uint32_t warnings : 32;
	CobTpdo4()
	{
		EMB_STATIC_ASSERT(sizeof(CobTpdo4) == 4);
		memset(this, 0, sizeof(CobTpdo4));
	}
};


extern ODEntry objectDictionary[];
extern const size_t objectDictionaryLen;


template <mcu::can::Peripheral::enum_type CanPeripheral, mcu::ipc::Mode::enum_type IpcMode, mcu::ipc::Role::enum_type IpcRole>
class Server : public IServer<CanPeripheral, IpcMode, IpcRole>
{
public:
	Server(NodeId nodeId, mcu::can::Module<CanPeripheral>* canModule, const IpcFlags& ipcFlags)
		: IServer<CanPeripheral, IpcMode, IpcRole>(nodeId, canModule, ipcFlags,
				IpcMode == mcu::ipc::Mode::Singlecore ? objectDictionary : NULL,
				IpcMode == mcu::ipc::Mode::Singlecore ? objectDictionaryLen : 0)
	{
		EMB_STATIC_ASSERT(IpcRole == mcu::ipc::Role::Primary);

		this->_registerTpdo(TpdoType::Tpdo1, 50);
		this->_registerTpdo(TpdoType::Tpdo2, 100);
		this->_registerTpdo(TpdoType::Tpdo3, 1000);
		this->_registerTpdo(TpdoType::Tpdo4, 100);
	}

	Server(const IpcFlags& ipcFlags)
		: IServer<CanPeripheral, IpcMode, IpcRole>(ipcFlags, objectDictionary, objectDictionaryLen)
	{
		EMB_STATIC_ASSERT(IpcMode == mcu::ipc::Mode::Dualcore);
		EMB_STATIC_ASSERT(IpcRole == mcu::ipc::Role::Secondary);
	}

protected:
	virtual can_payload _createTpdo1()
	{
		CobTpdo1 tpdo;

		tpdo.clock = mcu::chrono::SystemClock::now();

		return toPayload<CobTpdo1>(tpdo);
	}

	virtual can_payload _createTpdo2()
	{
		CobTpdo2 tpdo;

		tpdo.seconds = mcu::chrono::SystemClock::now() / 1000;
		tpdo.milliseconds = mcu::chrono::SystemClock::now() - 1000 * tpdo.seconds;

		return toPayload<CobTpdo2>(tpdo);
	}

	virtual can_payload _createTpdo3()
	{
		const emb::Array<unsigned int, 2> heartbeatValues = {0x55, 0xAA};
		static size_t heartbeatIndex = 0;

		CobTpdo3 tpdo;

		tpdo.heartbeat = heartbeatValues[heartbeatIndex];
		heartbeatIndex = (heartbeatIndex + 1) % heartbeatValues.size();

		return toPayload<CobTpdo3>(tpdo);
	}

	virtual can_payload _createTpdo4()
	{
		CobTpdo4 tpdo;

		tpdo.errors = SysLog::errors();
		tpdo.warnings = SysLog::warnings();

		return toPayload<CobTpdo4>(tpdo);
	}

	virtual void _handleRpdo1(const can_payload& data)
	{

	}

	virtual void _handleRpdo2(const can_payload& data)
	{

	}

	virtual void _handleRpdo3(const can_payload& data)
	{

	}

	virtual void _handleRpdo4(const can_payload& data)
	{

	}
};


} // namespace tests


/// @}
} // namespace ucanopen


