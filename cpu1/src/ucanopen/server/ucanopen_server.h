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
	NodeId _nodeId;
	mcu::can::Module<CanPeripheral>* _canModule;
	NmtState _nmtState;

	emb::Array<mcu::can::MessageObject, cobTypeCount> _messageObjects;

	/* HEARTBEAT */
private:
	impl::HeartbeatInfo _heartbeatInfo;

	/* TPDO */
private:
	emb::Array<impl::TpdoInfo, 4> _tpdoList;
protected:
	void _registerTpdo(TpdoType type, uint64_t period)
	{
		_tpdoList[type.underlying_value()].period = period;
	}

	virtual can_payload _createTpdo1() = 0;
	virtual can_payload _createTpdo2() = 0;
	virtual can_payload _createTpdo3() = 0;
	virtual can_payload _createTpdo4() = 0;

	/* RPDO */
private:
	emb::Array<impl::RpdoInfo, 4>* _rpdoList;
	emb::Array<mcu::ipc::Flag, 4> _rpdoReceived;
protected:
	void _registerRpdo(RpdoType type, uint64_t timeout, unsigned int id = 0)
	{
		_rpdoList[type.underlying_value()].timeout = timeout;
		if (id != 0)
		{
			CobType cob = toCobType(type);
			_messageObjects[cob.underlying_value()].frameId = id;
			_canModule->setupMessageObject(_messageObjects[cob.underlying_value()]);
		}
	}

	virtual void _handleRpdo1(const can_payload& data) = 0;
	virtual void _handleRpdo2(const can_payload& data) = 0;
	virtual void _handleRpdo3(const can_payload& data) = 0;
	virtual void _handleRpdo4(const can_payload& data) = 0;

	/* SDO */
private:
	ODEntry* _dictionary;
	size_t _dictionaryLen;
	mcu::ipc::Flag _rsdoReceived;
	mcu::ipc::Flag _tsdoReady;
	can_payload* _rsdoData;
	can_payload* _tsdoData;

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
		, _nodeId(nodeId)
		, _canModule(canModule)
		, _dictionary(objectDictionary)
		, _dictionaryLen(objectDictionaryLen)
	{
		EMB_STATIC_ASSERT(IpcRole == mcu::ipc::Role::Primary);
		_nmtState = NmtState::Initializing;

		_initAllocation();
		_initMessageObjects();

		for (size_t i = 1; i < cobTypeCount; ++i)	// count from 1 - skip dummy COB
		{
			_canModule->setupMessageObject(_messageObjects[i]);
		}

		// heartbeat setup
		_heartbeatInfo.period = 1000; // default HB period = 1000ms
		_heartbeatInfo.timepoint = mcu::chrono::SystemClock::now();

		// tpdo setup
		for (size_t i = 0; i < _tpdoList.size(); ++i)
		{
			_tpdoList[i].period = 0;
			_tpdoList[i].timepoint = mcu::chrono::SystemClock::now();
		}

		// rpdo setup
		for (size_t i = 0; i < _rpdoList->size(); ++i)
		{
			(*_rpdoList)[i].id = calculateCobId(toCobType(RpdoType(i)), _nodeId);
			(*_rpdoList)[i].timeout = 0;
			(*_rpdoList)[i].timepoint = mcu::chrono::SystemClock::now();
		}

		_rpdoReceived[RpdoType::Rpdo1] = ipcFlags.rpdo1Received;
		_rpdoReceived[RpdoType::Rpdo2] = ipcFlags.rpdo2Received;
		_rpdoReceived[RpdoType::Rpdo3] = ipcFlags.rpdo3Received;
		_rpdoReceived[RpdoType::Rpdo4] = ipcFlags.rpdo4Received;

		// sdo setup
		_initObjectDictionary();
		_rsdoReceived = ipcFlags.rsdoReceived;
		_tsdoReady = ipcFlags.tsdoReady;

		_canModule->registerInterruptCallback(onFrameReceived);

		_nmtState = NmtState::PreOperational;
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
		, _nodeId(NodeId(0))
		, _canModule(NULL)
		, _ipcFlags(ipcFlags)
		, _dictionary(objectDictionary)
		, _dictionaryLen(objectDictionaryLen)
	{
		EMB_STATIC_ASSERT(IpcMode != mcu::ipc::Mode::Singlecore);
		EMB_STATIC_ASSERT(IpcRole == mcu::ipc::Role::Secondary);

		_initAllocation();

		_rpdoReceived[RpdoType::Rpdo1] = ipcFlags.rpdo1Received;
		_rpdoReceived[RpdoType::Rpdo2] = ipcFlags.rpdo2Received;
		_rpdoReceived[RpdoType::Rpdo3] = ipcFlags.rpdo3Received;
		_rpdoReceived[RpdoType::Rpdo4] = ipcFlags.rpdo4Received;

		_initObjectDictionary();
		_rsdoReceived = ipcFlags.rsdoReceived;
		_tsdoReady = ipcFlags.tsdoReady;
	}

	/**
	 * @brief Enables server.
	 *
	 * @param (none)
	 * @return (none)
	 */
	void enable()
	{
		_canModule->enableInterrupts();
		_nmtState = NmtState::Operational;
	}

	/**
	 * @brief Disables server.
	 *
	 * @param (none)
	 * @return (none)
	 */
	void disable()
	{
		_canModule->disableInterrupts();
		_nmtState = NmtState::Stopped;
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
			_sendPeriodic();
			_handleRpdo();
			_handleRsdo();
			_sendTsdo();
			break;
		case mcu::ipc::Mode::Dualcore:
			switch (IpcRole)
			{
			case mcu::ipc::Role::Primary:
				_sendPeriodic();
				_sendTsdo();
				break;
			case mcu::ipc::Role::Secondary:
				_handleRpdo();
				_handleRsdo();
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
	void _sendPeriodic()
	{
		if (_heartbeatInfo.period != 0)
		{
			if (mcu::chrono::SystemClock::now() >= _heartbeatInfo.timepoint + _heartbeatInfo.period)
			{
				can_payload payload;
				payload[0] = _nmtState.underlying_value();
				_canModule->send(CobType::Heartbeat, payload.data, cobDataLen[CobType::Heartbeat]);
				_heartbeatInfo.timepoint = mcu::chrono::SystemClock::now();
			}
		}

		for (size_t i = 0; i < _tpdoList.size(); ++i)
		{
			if (_tpdoList[i].period == 0) continue;
			if (mcu::chrono::SystemClock::now() < _tpdoList[i].timepoint + _tpdoList[i].period) continue;

			can_payload payload;
			switch (i)
			{
			case 0:
				payload = _createTpdo1();
				break;
			case 1:
				payload = _createTpdo2();
				break;
			case 2:
				payload = _createTpdo3();
				break;
			case 3:
				payload = _createTpdo4();
				break;
			}

			CobType cob = toCobType(TpdoType(i));
			_canModule->send(cob.underlying_value(), payload.data, cobDataLen[cob.underlying_value()]);
			_tpdoList[i].timepoint = mcu::chrono::SystemClock::now();
		}
	}

	/**
	 * @brief
	 *
	 */
	void _handleRpdo()
	{
		if (_rpdoReceived[RpdoType::Rpdo1].isSet())
		{
			_handleRpdo1((*_rpdoList)[RpdoType::Rpdo1].data);
			_rpdoReceived[RpdoType::Rpdo1].reset();
		}

		if (_rpdoReceived[RpdoType::Rpdo2].isSet())
		{
			_handleRpdo2((*_rpdoList)[RpdoType::Rpdo2].data);
			_rpdoReceived[RpdoType::Rpdo2].reset();
		}

		if (_rpdoReceived[RpdoType::Rpdo3].isSet())
		{
			_handleRpdo3((*_rpdoList)[RpdoType::Rpdo3].data);
			_rpdoReceived[RpdoType::Rpdo3].reset();
		}

		if (_rpdoReceived[RpdoType::Rpdo4].isSet())
		{
			_handleRpdo4((*_rpdoList)[RpdoType::Rpdo4].data);
			_rpdoReceived[RpdoType::Rpdo4].reset();
		}
	}

	/**
	 * @brief
	 *
	 * @param data
	 */
	void _handleRsdo()
	{
		if (!_rsdoReceived.isSet()) return;

		ODAccessStatus status = ODAccessStatus::NoAccess;
		ODEntry* dictionaryEnd = _dictionary + _dictionaryLen;

		CobSdo rsdo = fromPayload<CobSdo>(*_rsdoData);
		CobSdo tsdo;

		_rsdoReceived.reset();

		const ODEntry* odEntry = emb::binary_find(_dictionary, dictionaryEnd,
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
			toPayload<CobSdo>(*_tsdoData, tsdo);
			_tsdoReady.local.set();
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
	void _sendTsdo()
	{
		if (!_tsdoReady.isSet()) return;
		_canModule->send(CobType::Tsdo, _tsdoData->data, cobDataLen[CobType::Tsdo]);
		_tsdoReady.reset();
	}

	/**
	 * @brief Initializes shared or non-shared objects.
	 *
	 */
	void _initAllocation()
	{
		switch (CanPeripheral)
		{
		case mcu::can::Peripheral::CanA:
			switch (IpcMode)
			{
			case mcu::ipc::Mode::Singlecore:
				_rpdoList = new emb::Array<impl::RpdoInfo, 4>;
				_rsdoData = new can_payload;
				_tsdoData = new can_payload;
				break;
			case mcu::ipc::Mode::Dualcore:
				_rpdoList = new(impl::cana_rpdoinfo_dualcore_alloc) emb::Array<impl::RpdoInfo, 4>;
				_rsdoData = new(impl::cana_rsdo_dualcore_alloc) can_payload;
				_tsdoData = new(impl::cana_tsdo_dualcore_alloc) can_payload;
				break;
			}
			break;
		case mcu::can::Peripheral::CanB:
			switch (IpcMode)
			{
			case mcu::ipc::Mode::Singlecore:
				_rpdoList = new emb::Array<impl::RpdoInfo, 4>;
				_rsdoData = new can_payload;
				_tsdoData = new can_payload;
				break;
			case mcu::ipc::Mode::Dualcore:
				_rpdoList = new(impl::canb_rpdoinfo_dualcore_alloc) emb::Array<impl::RpdoInfo, 4>;
				_rsdoData = new(impl::canb_rsdo_dualcore_alloc) can_payload;
				_tsdoData = new(impl::canb_tsdo_dualcore_alloc) can_payload;
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
	void _initMessageObjects()
	{
		for (size_t i = 0; i < cobTypeCount; ++i)
		{
			_messageObjects[i].objId = i;
			_messageObjects[i].frameId = calculateCobId(CobType(i), _nodeId);
			_messageObjects[i].frameType = CAN_MSG_FRAME_STD;
			_messageObjects[i].frameIdMask = 0;
			_messageObjects[i].dataLen = cobDataLen[i];
		}

		_messageObjects[CobType::Emcy].objType
				= _messageObjects[CobType::Tpdo1].objType
				= _messageObjects[CobType::Tpdo2].objType
				= _messageObjects[CobType::Tpdo3].objType
				= _messageObjects[CobType::Tpdo4].objType
				= _messageObjects[CobType::Tsdo].objType
				= _messageObjects[CobType::Heartbeat].objType
				= CAN_MSG_OBJ_TYPE_TX;

		_messageObjects[CobType::Nmt].objType
				= _messageObjects[CobType::Sync].objType
				= _messageObjects[CobType::Time].objType
				= _messageObjects[CobType::Rpdo1].objType
				= _messageObjects[CobType::Rpdo2].objType
				= _messageObjects[CobType::Rpdo3].objType
				= _messageObjects[CobType::Rpdo4].objType
				= _messageObjects[CobType::Rsdo].objType
				= CAN_MSG_OBJ_TYPE_RX;

		_messageObjects[CobType::Emcy].flags
				= _messageObjects[CobType::Tpdo1].flags
				= _messageObjects[CobType::Tpdo2].flags
				= _messageObjects[CobType::Tpdo3].flags
				= _messageObjects[CobType::Tpdo4].flags
				= _messageObjects[CobType::Tsdo].flags
				= _messageObjects[CobType::Heartbeat].flags
				= CAN_MSG_OBJ_NO_FLAGS;

		_messageObjects[CobType::Nmt].flags
				= _messageObjects[CobType::Sync].flags
				= _messageObjects[CobType::Time].flags
				= _messageObjects[CobType::Rpdo1].flags
				= _messageObjects[CobType::Rpdo2].flags
				= _messageObjects[CobType::Rpdo3].flags
				= _messageObjects[CobType::Rpdo4].flags
				= _messageObjects[CobType::Rsdo].flags
				= CAN_MSG_OBJ_RX_INT_ENABLE;
	}

	/**
	 * @brief
	 *
	 */
	void _initObjectDictionary()
	{
		if (IpcMode == mcu::ipc::Mode::Dualcore
				&& IpcRole == mcu::ipc::Role::Primary)
		{
			assert(_dictionary == NULL);
			return;
		}

		assert(_dictionary != NULL);

		std::sort(_dictionary, _dictionary + _dictionaryLen);

		// Check OBJECT DICTIONARY correctness
		for (size_t i = 0; i < _dictionaryLen; ++i)
		{
			// OD is sorted
			if (i < (_dictionaryLen - 1))
			{
				assert(_dictionary[i] < _dictionary[i+1]);
			}

			for (size_t j = i+1; j < _dictionaryLen; ++j)
			{
				// no od-entries with equal {index, subinex}
				assert((_dictionary[i].key.index != _dictionary[j].key.index)
					|| (_dictionary[i].key.subindex != _dictionary[j].key.subindex));

				// no od-entries with equal {category, subcategory, name}
				bool categoryEqual = ((strcmp(_dictionary[i].value.category, _dictionary[j].value.category) == 0) ? true : false);
				bool subcategoryEqual = ((strcmp(_dictionary[i].value.subcategory, _dictionary[j].value.subcategory) == 0) ? true : false);
				bool nameEqual = ((strcmp(_dictionary[i].value.name, _dictionary[j].value.name) == 0) ? true : false);
				assert(!categoryEqual || !subcategoryEqual || !nameEqual);
			}

			if (_dictionary[i].hasReadAccess())
			{
				assert((_dictionary[i].value.readAccessFunc != OD_NO_INDIRECT_READ_ACCESS)
						|| (_dictionary[i].value.dataPtr != OD_NO_DIRECT_ACCESS));
			}
			else
			{
				assert(_dictionary[i].value.readAccessFunc == OD_NO_INDIRECT_READ_ACCESS
						&& (_dictionary[i].value.dataPtr == OD_NO_DIRECT_ACCESS));
			}

			if (_dictionary[i].hasWriteAccess())
			{
				assert(_dictionary[i].value.writeAccessFunc != OD_NO_INDIRECT_WRITE_ACCESS
						|| (_dictionary[i].value.dataPtr != OD_NO_DIRECT_ACCESS));
			}
			else
			{
				assert(_dictionary[i].value.writeAccessFunc == OD_NO_INDIRECT_WRITE_ACCESS
						&& (_dictionary[i].value.dataPtr == OD_NO_DIRECT_ACCESS));
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
			(*server->_rpdoList)[rpdoIdx].timepoint = mcu::chrono::SystemClock::now();

			if (server->_rpdoReceived[rpdoIdx].local.isSet())
			{
				SysLog::setWarning(sys::Warning::CanBusOverrun);
			}
			else
			{
				// there is no unprocessed RPDO of this type
				canModule->recv(interruptCause, (*server->_rpdoList)[rpdoIdx].data.data);
				server->_rpdoReceived[rpdoIdx].local.set();
			}
		}

		case CobType::Rsdo:
		{
			SysLog::resetWarning(sys::Warning::CanBusError);
			if (server->_rsdoReceived.local.isSet()
					|| server->_tsdoReady.isSet())
			{
				SysLog::setWarning(sys::Warning::CanBusOverrun);
				SysLog::addMessage(sys::Message::CanSdoRequestLost);
			}
			else
			{
				canModule->recv(interruptCause, server->_rsdoData->data);
				server->_rsdoReceived.local.set();
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


