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
#include "mcu_f2837xd/chrono/mcu_chrono.h"


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
	NodeId m_nodeId;
	mcu::can::IModule* m_canModule;
	NmtState m_nmtState;

	emb::Array<mcu::can::MessageObject, cobTypeCount> m_messageObjects;

	IpcFlags m_ipcFlags;

	/* HEARTBEAT */
private:
	struct HeartbeatInfo
	{
		uint64_t period;
		uint64_t timepoint;
	};
	HeartbeatInfo m_heartbeatInfo;

	/* TPDO */
private:
	struct TpdoInfo
	{
		uint64_t period;
		uint64_t timepoint;
	};
	emb::Array<TpdoInfo, 4> m_tpdoList;
protected:
	void registerTpdo(TpdoType type, uint64_t period)
	{
		m_tpdoList[type.underlying_value()].period = period;
	}

	virtual can_payload createTpdo1() {}
	virtual can_payload createTpdo2() {}
	virtual can_payload createTpdo3() {}
	virtual can_payload createTpdo4() {}

	/* RPDO */
private:
	struct RpdoInfo
	{
		unsigned int id;
		uint64_t timeout;
		uint64_t timepoint;
		bool isOnSchedule;
		bool isUnprocessed;
	};
	emb::Array<RpdoInfo, 4> m_rpdoList;
protected:
	void registerRpdo(RpdoType type, uint64_t timeout, unsigned int id = 0)
	{
		m_rpdoList[type.underlying_value()].timeout = timeout;
		if (id != 0)
		{
			CobType cob = toCobType(type);
			m_messageObjects[cob.underlying_value()].frameId = id;
			m_canModule->setupMessageObject(m_messageObjects[cob.underlying_value()]);
		}
	}

	virtual void handleRpdo1(can_payload data) {}
	virtual void handleRpdo2(can_payload data) {}
	virtual void handleRpdo3(can_payload data) {}
	virtual void handleRpdo4(can_payload data) {}

public:

	IServer(NodeId nodeId, mcu::can::IModule* canModule, const IpcFlags& ipcFlags)
		: emb::c28x::interrupt_invoker<IServer<IpcMode, IpcRole> >(this)
		, m_nodeId(nodeId)
		, m_canModule(canModule)
		, m_ipcFlags(ipcFlags)
	{
		EMB_STATIC_ASSERT(IpcRole == mcu::ipc::Role::Primary);
		m_nmtState = NmtState::Initializing;

		initMessageObjects();

		for (size_t i = 1; i < cobTypeCount; ++i)	// count from 1 - skip dummy COB
		{
			m_canModule->setupMessageObject(m_messageObjects[i]);
		}

		m_heartbeatInfo.period = 1000; // default HB period = 1000ms
		for (size_t i = 0; i < m_tpdoList.size(); ++i)
		{
			m_tpdoList[i].period = 0;
			m_tpdoList[i].timepoint = mcu::chrono::SystemClock::now();
		}
		for (size_t i = 0; i < m_rpdoList.size(); ++i)
		{
			m_rpdoList[i].id = calculateCobId(toCobType(RpdoType(i)), m_nodeId.value());
			m_rpdoList[i].timeout = 0;
			m_rpdoList[i].timepoint = mcu::chrono::SystemClock::now();
			m_rpdoList[i].isOnSchedule = false;
			m_rpdoList[i].isUnprocessed = false;
		}

		m_canModule->registerInterruptCallback(onFrameReceived);

		m_nmtState = NmtState::PreOperational;
	}


	IServer(const IpcFlags& ipcFlags)
		: emb::c28x::interrupt_invoker<IServer<IpcMode, IpcRole> >(this)
		, m_nodeId(NodeId(0))
		, m_canModule(NULL)
		, m_ipcFlags(ipcFlags)
	{
		EMB_STATIC_ASSERT(IpcMode != mcu::ipc::Mode::Singlecore);
		EMB_STATIC_ASSERT(IpcRole == mcu::ipc::Role::Secondary);
	}

	/**
	 * @brief Enables server.
	 *
	 * @param (none)
	 * @return (none)
	 */
	void enable()
	{
		m_canModule->enableInterrupts();
		m_nmtState = NmtState::Operational;
	}

	/**
	 * @brief Disables server.
	 *
	 * @param (none)
	 * @return (none)
	 */
	void disable()
	{
		m_canModule->disableRxInterrupt();
		m_nmtState = NmtState::Stopped;
	}

	/**
	 * @brief Runs all server operations.
	 *
	 * @param (none)
	 * @return (none)
	 */
	void run()
	{
		switch (IpcMode)
		{
		case mcu::ipc::Mode::Singlecore:
			sendPeriodic();
			//processRawRdo();
			//m_rpdoService->respondToProcessedRpdo();
			//m_sdoService->processRequest();
			//sendSdoResponse();
			break;
		case mcu::ipc::Mode::Dualcore:
			//switch (Mode)
			//{
			//case emb::MODE_MASTER:
			//	processRawRdo();
			//	runPeriodicTasks();
			//	sendSdoResponse();
			//	break;
			//case emb::MODE_SLAVE:
			//	m_rpdoService->respondToProcessedRpdo();
			//	m_sdoService->processRequest();
			//	break;
			//}
			break;
		}
	}

protected:
	void sendPeriodic()
	{
		if (m_heartbeatInfo.period != 0)
		{
			if (mcu::chrono::SystemClock::now() >= m_heartbeatInfo.timepoint + m_heartbeatInfo.period)
			{
				can_payload payload;
				payload[0] = m_nmtState.underlying_value();
				m_canModule->send(CobType::Heartbeat, payload.data, cobDataLen[m_nmtState.underlying_value()]);
				m_heartbeatInfo.timepoint = mcu::chrono::SystemClock::now();
			}
		}

		for (size_t i = 0; i < m_tpdoList.size(); ++i)
		{
			if (m_tpdoList[i].period == 0) continue;
			if (mcu::chrono::SystemClock::now() < m_tpdoList[i].timepoint + m_tpdoList[i].period) continue;

			can_payload payload;
			switch (i)
			{
			case 0:
				payload = createTpdo1();
				break;
			case 1:
				payload = createTpdo2();
				break;
			case 2:
				payload = createTpdo3();
				break;
			case 3:
				payload = createTpdo4();
				break;
			}

			CobType cob = toCobType(TpdoType(i));
			m_canModule->send(cob.underlying_value(), payload.data, cobDataLen[cob.underlying_value()]);
			m_tpdoList[i].timepoint = mcu::chrono::SystemClock::now();
		}
	}








































	/**
	 * @brief Initializes message objects.
	 *
	 * @param (none)
	 * @return (none)
	 */
	void initMessageObjects()
	{
		for (size_t i = 0; i < cobTypeCount; ++i)
		{
			m_messageObjects[i].objId = i;
			m_messageObjects[i].frameId = calculateCobId(CobType(i), m_nodeId.value());
			m_messageObjects[i].frameType = CAN_MSG_FRAME_STD;
			m_messageObjects[i].frameIdMask = 0;
			m_messageObjects[i].dataLen = cobDataLen[i];
		}

		m_messageObjects[CobType::Emcy].objType
				= m_messageObjects[CobType::Tpdo1].objType
				= m_messageObjects[CobType::Tpdo2].objType
				= m_messageObjects[CobType::Tpdo3].objType
				= m_messageObjects[CobType::Tpdo4].objType
				= m_messageObjects[CobType::Tsdo].objType
				= m_messageObjects[CobType::Heartbeat].objType
				= CAN_MSG_OBJ_TYPE_TX;

		m_messageObjects[CobType::Nmt].objType
				= m_messageObjects[CobType::Sync].objType
				= m_messageObjects[CobType::Time].objType
				= m_messageObjects[CobType::Rpdo1].objType
				= m_messageObjects[CobType::Rpdo2].objType
				= m_messageObjects[CobType::Rpdo3].objType
				= m_messageObjects[CobType::Rpdo4].objType
				= m_messageObjects[CobType::Rsdo].objType
				= CAN_MSG_OBJ_TYPE_RX;

		m_messageObjects[CobType::Emcy].flags
				= m_messageObjects[CobType::Tpdo1].flags
				= m_messageObjects[CobType::Tpdo2].flags
				= m_messageObjects[CobType::Tpdo3].flags
				= m_messageObjects[CobType::Tpdo4].flags
				= m_messageObjects[CobType::Tsdo].flags
				= m_messageObjects[CobType::Heartbeat].flags
				= CAN_MSG_OBJ_NO_FLAGS;

		m_messageObjects[CobType::Nmt].flags
				= m_messageObjects[CobType::Sync].flags
				= m_messageObjects[CobType::Time].flags
				= m_messageObjects[CobType::Rpdo1].flags
				= m_messageObjects[CobType::Rpdo2].flags
				= m_messageObjects[CobType::Rpdo3].flags
				= m_messageObjects[CobType::Rpdo4].flags
				= m_messageObjects[CobType::Rsdo].flags
				= CAN_MSG_OBJ_RX_INT_ENABLE;
	}

	/**
	 * @brief
	 *
	 */
	static void onFrameReceived(mcu::can::IModule* canModule, uint32_t interruptCause, uint16_t status)
	{

	}
};










/// @}
}


