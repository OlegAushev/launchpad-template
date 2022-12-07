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


#include <new>
#include <algorithm>
#include "../ucanopen_def.h"
#include "mcu_f2837xd/ipc/mcu_ipc.h"
#include "mcu_f2837xd/can/mcu_can.h"
#include "mcu_f2837xd/chrono/mcu_chrono.h"
#include "sys/syslog/syslog.h"


namespace ucanopen {
/// @addtogroup ucanopen_server
/// @{


/**
 * @brief IPC flags.
 *
 */
struct IpcFlags
{
	mcu::ipc::Flag rpdo1Received;
	mcu::ipc::Flag rpdo2Received;
	mcu::ipc::Flag rpdo3Received;
	mcu::ipc::Flag rpdo4Received;
	mcu::ipc::Flag rsdoReceived;
	mcu::ipc::Flag tsdoReady;
};


namespace impl {


struct HeartbeatInfo
{
	uint64_t period;
	uint64_t timepoint;
};


struct TpdoInfo
{
	uint64_t period;
	uint64_t timepoint;
};


struct RpdoInfo
{
	unsigned int id;
	uint64_t timeout;
	uint64_t timepoint;
	bool isOnSchedule;
	can_payload data;
};


extern unsigned char cana_rpdoinfo_dualcore_alloc[sizeof(emb::Array<impl::RpdoInfo, 4>)];
extern unsigned char canb_rpdoinfo_dualcore_alloc[sizeof(emb::Array<impl::RpdoInfo, 4>)];

extern unsigned char cana_rsdo_dualcore_alloc[sizeof(can_payload)];
extern unsigned char canb_rsdo_dualcore_alloc[sizeof(can_payload)];

extern unsigned char cana_tsdo_dualcore_alloc[sizeof(can_payload)];
extern unsigned char canb_tsdo_dualcore_alloc[sizeof(can_payload)];


} // namespace impl


/**
 * @brief uCANopen server interface.
 */
template <mcu::can::Peripheral::enum_type CanPeripheral, mcu::ipc::Mode::enum_type IpcMode, mcu::ipc::Role::enum_type IpcRole>
class IServer : public emb::c28x::interrupt_invoker<IServer<CanPeripheral, IpcMode, IpcRole> >
{
private:
	NodeId m_nodeId;
	mcu::can::Module<CanPeripheral>* m_canModule;
	NmtState m_nmtState;

	emb::Array<mcu::can::MessageObject, cobTypeCount> m_messageObjects;

	/* HEARTBEAT */
private:
	impl::HeartbeatInfo m_heartbeatInfo;

	/* TPDO */
private:
	emb::Array<impl::TpdoInfo, 4> m_tpdoList;
protected:
	void registerTpdo(TpdoType type, uint64_t period)
	{
		m_tpdoList[type.underlying_value()].period = period;
	}

	virtual can_payload createTpdo1() = 0;
	virtual can_payload createTpdo2() = 0;
	virtual can_payload createTpdo3() = 0;
	virtual can_payload createTpdo4() = 0;

	/* RPDO */
private:
	emb::Array<impl::RpdoInfo, 4>* m_rpdoList;
	emb::Array<mcu::ipc::Flag, 4> m_rpdoReceived;
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

	virtual void handleRpdo1(const can_payload& data) = 0;
	virtual void handleRpdo2(const can_payload& data) = 0;
	virtual void handleRpdo3(const can_payload& data) = 0;
	virtual void handleRpdo4(const can_payload& data) = 0;

	/* SDO */
private:
	ODEntry* m_dictionary;
	size_t m_dictionaryLen;
	mcu::ipc::Flag m_rsdoReceived;
	mcu::ipc::Flag m_tsdoReady;
	can_payload* m_rsdoData;
	can_payload* m_tsdoData;

public:
	/**
	 * @brief
	 *
	 * @param nodeId
	 * @param canModule
	 * @param ipcFlags
	 * @param objectDictionary
	 * @param objectDictionaryLen
	 */
	IServer(NodeId nodeId, mcu::can::Module<CanPeripheral>* canModule, const IpcFlags& ipcFlags,
			ODEntry* objectDictionary, size_t objectDictionaryLen)
		: emb::c28x::interrupt_invoker<IServer<CanPeripheral, IpcMode, IpcRole> >(this)
		, m_nodeId(nodeId)
		, m_canModule(canModule)
		, m_dictionary(objectDictionary)
		, m_dictionaryLen(objectDictionaryLen)
	{
		EMB_STATIC_ASSERT(IpcRole == mcu::ipc::Role::Primary);
		m_nmtState = NmtState::Initializing;

		initAllocation();
		initMessageObjects();

		for (size_t i = 1; i < cobTypeCount; ++i)	// count from 1 - skip dummy COB
		{
			m_canModule->setupMessageObject(m_messageObjects[i]);
		}

		// heartbeat setup
		m_heartbeatInfo.period = 1000; // default HB period = 1000ms
		m_heartbeatInfo.timepoint = mcu::chrono::SystemClock::now();

		// tpdo setup
		for (size_t i = 0; i < m_tpdoList.size(); ++i)
		{
			m_tpdoList[i].period = 0;
			m_tpdoList[i].timepoint = mcu::chrono::SystemClock::now();
		}

		// rpdo setup
		for (size_t i = 0; i < m_rpdoList->size(); ++i)
		{
			(*m_rpdoList)[i].id = calculateCobId(toCobType(RpdoType(i)), m_nodeId.value());
			(*m_rpdoList)[i].timeout = 0;
			(*m_rpdoList)[i].timepoint = mcu::chrono::SystemClock::now();
			(*m_rpdoList)[i].isOnSchedule = false;
		}

		m_rpdoReceived[RpdoType::Rpdo1] = ipcFlags.rpdo1Received;
		m_rpdoReceived[RpdoType::Rpdo2] = ipcFlags.rpdo2Received;
		m_rpdoReceived[RpdoType::Rpdo3] = ipcFlags.rpdo3Received;
		m_rpdoReceived[RpdoType::Rpdo4] = ipcFlags.rpdo4Received;

		// sdo setup
		initObjectDictionary();
		m_rsdoReceived = ipcFlags.rsdoReceived;
		m_tsdoReady = ipcFlags.tsdoReady;

		m_canModule->registerInterruptCallback(onFrameReceived);

		m_nmtState = NmtState::PreOperational;
	}

	/**
	 * @brief
	 *
	 * @param ipcFlags
	 * @param objectDictionary
	 * @param objectDictionaryLen
	 */
	IServer(const IpcFlags& ipcFlags, ODEntry* objectDictionary, size_t objectDictionaryLen)
		: emb::c28x::interrupt_invoker<IServer<CanPeripheral, IpcMode, IpcRole> >(this)
		, m_nodeId(NodeId(0))
		, m_canModule(NULL)
		, m_ipcFlags(ipcFlags)
		, m_dictionary(objectDictionary)
		, m_dictionaryLen(objectDictionaryLen)
	{
		EMB_STATIC_ASSERT(IpcMode != mcu::ipc::Mode::Singlecore);
		EMB_STATIC_ASSERT(IpcRole == mcu::ipc::Role::Secondary);

		initAllocation();

		m_rpdoReceived[RpdoType::Rpdo1] = ipcFlags.rpdo1Received;
		m_rpdoReceived[RpdoType::Rpdo2] = ipcFlags.rpdo2Received;
		m_rpdoReceived[RpdoType::Rpdo3] = ipcFlags.rpdo3Received;
		m_rpdoReceived[RpdoType::Rpdo4] = ipcFlags.rpdo4Received;

		initObjectDictionary();
		m_rsdoReceived = ipcFlags.rsdoReceived;
		m_tsdoReady = ipcFlags.tsdoReady;
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
		m_canModule->disableInterrupts();
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
			handleRpdo();
			handleRsdo();
			sendTsdo();
			break;
		case mcu::ipc::Mode::Dualcore:
			switch (IpcRole)
			{
			case mcu::ipc::Role::Primary:
				sendPeriodic();
				sendTsdo();
				break;
			case mcu::ipc::Role::Secondary:
				handleRpdo();
				handleRsdo();
				break;
			}
			break;
		}
	}

private:
	/**
	 * @brief
	 *
	 */
	void sendPeriodic()
	{
		if (m_heartbeatInfo.period != 0)
		{
			if (mcu::chrono::SystemClock::now() >= m_heartbeatInfo.timepoint + m_heartbeatInfo.period)
			{
				can_payload payload;
				payload[0] = m_nmtState.underlying_value();
				m_canModule->send(CobType::Heartbeat, payload.data, cobDataLen[CobType::Heartbeat]);
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
	 * @brief
	 *
	 */
	void handleRpdo()
	{
		if (m_rpdoReceived[RpdoType::Rpdo1].isSet())
		{
			handleRpdo1((*m_rpdoList)[RpdoType::Rpdo1].data);
			m_rpdoReceived[RpdoType::Rpdo1].reset();
		}

		if (m_rpdoReceived[RpdoType::Rpdo2].isSet())
		{
			handleRpdo2((*m_rpdoList)[RpdoType::Rpdo2].data);
			m_rpdoReceived[RpdoType::Rpdo2].reset();
		}

		if (m_rpdoReceived[RpdoType::Rpdo3].isSet())
		{
			handleRpdo3((*m_rpdoList)[RpdoType::Rpdo3].data);
			m_rpdoReceived[RpdoType::Rpdo3].reset();
		}

		if (m_rpdoReceived[RpdoType::Rpdo4].isSet())
		{
			handleRpdo4((*m_rpdoList)[RpdoType::Rpdo4].data);
			m_rpdoReceived[RpdoType::Rpdo4].reset();
		}
	}

	/**
	 * @brief
	 *
	 * @param data
	 */
	void handleRsdo()
	{
		if (!m_rsdoReceived.isSet()) return;

		ODAccessStatus status = ODAccessStatus::NoAccess;
		ODEntry* dictionaryEnd = m_dictionary + m_dictionaryLen;

		CobSdo rsdo = fromPayload<CobSdo>(*m_rsdoData);
		CobSdo tsdo;

		m_rsdoReceived.reset();

		const ODEntry* odEntry = emb::binary_find(m_dictionary, dictionaryEnd,
				ODEntryKeyAux(rsdo.index, rsdo.subindex));

		if (odEntry == dictionaryEnd) return;	// OD-entry not found

		if (rsdo.cs == cs_codes::sdoCcsRead)
		{
			if ((odEntry->value.dataPtr != OD_NO_DIRECT_ACCESS) && odEntry->hasReadAccess())
			{
				memcpy(&tsdo.data.u32, odEntry->value.dataPtr, odEntryDataSizes[odEntry->value.dataType]);
				status = ODAccessStatus::Success;
			}
			else
			{
				status = odEntry->value.readAccessFunc(tsdo.data);
			}
		}
		else if (rsdo.cs == cs_codes::sdoCcsWrite)
		{
			if ((odEntry->value.dataPtr != OD_NO_DIRECT_ACCESS) && odEntry->hasWriteAccess())
			{
				memcpy(odEntry->value.dataPtr, &rsdo.data.u32, odEntryDataSizes[odEntry->value.dataType]);
				status = ODAccessStatus::Success;
			}
			else
			{
				status = odEntry->value.writeAccessFunc(rsdo.data);
			}
		}
		else
		{
			return;
		}

		switch (status.underlying_value())
		{
		case ODAccessStatus::Success:
			tsdo.index = rsdo.index;
			tsdo.subindex = rsdo.subindex;
			if (rsdo.cs == cs_codes::sdoCcsRead)
			{
				tsdo.cs = cs_codes::sdoScsRead;		// read/upload response
				tsdo.expeditedTransfer = 1;
				tsdo.dataSizeIndicated = 1;
				tsdo.dataEmptyBytes = 0;
			}
			else if (rsdo.cs == cs_codes::sdoCcsWrite)
			{
				tsdo.cs = cs_codes::sdoScsWrite;	// write/download response
				tsdo.expeditedTransfer = 0;
				tsdo.dataSizeIndicated = 0;
				tsdo.dataEmptyBytes = 0;
			}
			else
			{
				return;
			}
			toPayload<CobSdo>(*m_tsdoData, tsdo);
			m_tsdoReady.local.set();
			break;

		case ODAccessStatus::Fail:
		case ODAccessStatus::NoAccess:
			return;
		}
	}

	/**
	 * @brief
	 *
	 */
	void sendTsdo()
	{
		if (!m_tsdoReady.isSet()) return;
		m_canModule->send(CobType::Tsdo, m_tsdoData->data, cobDataLen[CobType::Tsdo]);
		m_tsdoReady.reset();
	}

	/**
	 * @brief Initializes shared or non-shared objects.
	 *
	 */
	void initAllocation()
	{
		switch (CanPeripheral)
		{
		case mcu::can::Peripheral::CanA:
			switch (IpcMode)
			{
			case mcu::ipc::Mode::Singlecore:
				m_rpdoList = new emb::Array<impl::RpdoInfo, 4>;
				m_rsdoData = new can_payload;
				m_tsdoData = new can_payload;
				break;
			case mcu::ipc::Mode::Dualcore:
				m_rpdoList = new(impl::cana_rpdoinfo_dualcore_alloc) emb::Array<impl::RpdoInfo, 4>;
				m_rsdoData = new(impl::cana_rsdo_dualcore_alloc) can_payload;
				m_tsdoData = new(impl::cana_tsdo_dualcore_alloc) can_payload;
				break;
			}
			break;
		case mcu::can::Peripheral::CanB:
			switch (IpcMode)
			{
			case mcu::ipc::Mode::Singlecore:
				m_rpdoList = new emb::Array<impl::RpdoInfo, 4>;
				m_rsdoData = new can_payload;
				m_tsdoData = new can_payload;
				break;
			case mcu::ipc::Mode::Dualcore:
				m_rpdoList = new(impl::canb_rpdoinfo_dualcore_alloc) emb::Array<impl::RpdoInfo, 4>;
				m_rsdoData = new(impl::canb_rsdo_dualcore_alloc) can_payload;
				m_tsdoData = new(impl::canb_tsdo_dualcore_alloc) can_payload;
				break;
			}
			break;
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
	void initObjectDictionary()
	{
		if (IpcMode == mcu::ipc::Mode::Dualcore
				&& IpcRole == mcu::ipc::Role::Primary)
		{
			assert(m_dictionary == NULL);
			return;
		}

		assert(m_dictionary != NULL);

		std::sort(m_dictionary, m_dictionary + m_dictionaryLen);

		// Check OBJECT DICTIONARY correctness
		for (size_t i = 0; i < m_dictionaryLen; ++i)
		{
			// OD is sorted
			if (i < (m_dictionaryLen - 1))
			{
				assert(m_dictionary[i] < m_dictionary[i+1]);
			}

			for (size_t j = i+1; j < m_dictionaryLen; ++j)
			{
				// no od-entries with equal {index, subinex}
				assert((m_dictionary[i].key.index != m_dictionary[j].key.index)
					|| (m_dictionary[i].key.subindex != m_dictionary[j].key.subindex));

				// no od-entries with equal {category, subcategory, name}
				bool categoryEqual = ((strcmp(m_dictionary[i].value.category, m_dictionary[j].value.category) == 0) ? true : false);
				bool subcategoryEqual = ((strcmp(m_dictionary[i].value.subcategory, m_dictionary[j].value.subcategory) == 0) ? true : false);
				bool nameEqual = ((strcmp(m_dictionary[i].value.name, m_dictionary[j].value.name) == 0) ? true : false);
				assert(!categoryEqual || !subcategoryEqual || !nameEqual);
			}

			if (m_dictionary[i].hasReadAccess())
			{
				assert((m_dictionary[i].value.readAccessFunc != OD_NO_INDIRECT_READ_ACCESS)
						|| (m_dictionary[i].value.dataPtr != OD_NO_DIRECT_ACCESS));
			}
			else
			{
				assert(m_dictionary[i].value.readAccessFunc == OD_NO_INDIRECT_READ_ACCESS
						&& (m_dictionary[i].value.dataPtr == OD_NO_DIRECT_ACCESS));
			}

			if (m_dictionary[i].hasWriteAccess())
			{
				assert(m_dictionary[i].value.writeAccessFunc != OD_NO_INDIRECT_WRITE_ACCESS
						|| (m_dictionary[i].value.dataPtr != OD_NO_DIRECT_ACCESS));
			}
			else
			{
				assert(m_dictionary[i].value.writeAccessFunc == OD_NO_INDIRECT_WRITE_ACCESS
						&& (m_dictionary[i].value.dataPtr == OD_NO_DIRECT_ACCESS));
			}
		}
	}


	/**
	 * @brief
	 *
	 */
	static void onFrameReceived(mcu::can::Module<CanPeripheral>* canModule, uint32_t interruptCause, uint16_t status)
	{
		IServer<CanPeripheral, IpcMode, IpcRole>* server = IServer<CanPeripheral, IpcMode, IpcRole>::instance();

		switch (interruptCause)
		{
		case CAN_INT_INT0ID_STATUS:
			switch (status)
			{
			case CAN_STATUS_PERR:
			case CAN_STATUS_BUS_OFF:
			case CAN_STATUS_EWARN:
			case CAN_STATUS_LEC_BIT1:
			case CAN_STATUS_LEC_BIT0:
			case CAN_STATUS_LEC_CRC:
				SysLog::setWarning(sys::Warning::CanBusError);
				break;
			default:
				break;
			}
			break;

		case CobType::Rpdo1:
		case CobType::Rpdo2:
		case CobType::Rpdo3:
		case CobType::Rpdo4:
		{
			SysLog::resetWarning(sys::Warning::CanBusError);

			size_t rpdoIdx = (interruptCause - static_cast<size_t>(CobType::Rpdo1)) / 2;

			(*server->m_rpdoList)[rpdoIdx].timepoint = mcu::chrono::SystemClock::now();
			(*server->m_rpdoList)[rpdoIdx].isOnSchedule = true;

			if (server->m_rpdoReceived[rpdoIdx].local.isSet())
			{
				SysLog::setWarning(sys::Warning::CanBusOverrun);
			}
			else
			{
				// there is no unprocessed RPDO of this type
				canModule->recv(interruptCause, (*server->m_rpdoList)[rpdoIdx].data.data);
				server->m_rpdoReceived[rpdoIdx].local.set();
			}
		}

		case CobType::Rsdo:
		{
			SysLog::resetWarning(sys::Warning::CanBusError);
			if (server->m_rsdoReceived.local.isSet()
					|| server->m_tsdoReady.isSet())
			{
				SysLog::setWarning(sys::Warning::CanBusOverrun);
				SysLog::addMessage(sys::Message::CanSdoRequestLost);
			}
			else
			{
				canModule->recv(interruptCause, server->m_rsdoData->data);
				server->m_rsdoReceived.local.set();
			}
			break;
		}

		default:
			break;
		}
	}
};


/// @}
}


