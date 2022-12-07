/**
 * @defgroup ucanopen uCANopen
 * 
 * @file ucanopen_def.h
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


#include <cstring>
#include "emb/emb_core.h"
#include "emb/emb_array.h"


namespace ucanopen {
/// @addtogroup ucanopen
/// @{


/// CAN payload
typedef emb::Array<uint16_t, 8> can_payload;


/**
 * @brief
 *
 * @tparam T
 * @param message
 * @return
 */
template <typename T>
inline can_payload toPayload(T message)
{
	EMB_STATIC_ASSERT(sizeof(T) <= 4);
	can_payload payload;
	payload.fill(0);
	emb::c28x::to_bytes(payload.data, message);
	return payload;
}


/**
 * @brief
 *
 * @tparam T
 * @param payload
 * @param message
 * @return
 */
template <typename T>
inline void toPayload(can_payload& payload, T message)
{
	EMB_STATIC_ASSERT(sizeof(T) <= 4);
	payload.fill(0);
	emb::c28x::to_bytes(payload.data, message);
}


/**
 * @brief
 *
 * @tparam T
 * @param payload
 * @return
 */
template <typename T>
inline T fromPayload(const can_payload& payload)
{
	EMB_STATIC_ASSERT(sizeof(T) <= 4);
	T message;
	emb::c28x::from_bytes(message, payload.data);
	return message;
}


/**
 * @brief Node ID class.
 */
class NodeId
{
private:
	unsigned int m_value;
public:
	explicit NodeId(unsigned int value) : m_value(value) {}
	unsigned int value() const { return m_value; }
};


/**
 * @brief NMT state.
 *
 */
SCOPED_ENUM_DECLARE_BEGIN(NmtState)
{
	Initializing = 0x00,
	Stopped = 0x04,
	Operational = 0x05,
	PreOperational = 0x7F
}
SCOPED_ENUM_DECLARE_END(NmtState)



SCOPED_ENUM_DECLARE_BEGIN(CobType)
{
	Dummy,
	Nmt,
	Sync,
	Emcy,
	Time,
	Tpdo1,
	Rpdo1,
	Tpdo2,
	Rpdo2,
	Tpdo3,
	Rpdo3,
	Tpdo4,
	Rpdo4,
	Tsdo,
	Rsdo,
	Heartbeat
}
SCOPED_ENUM_DECLARE_END(CobType)


const size_t cobTypeCount = 16;


const emb::Array<uint32_t, cobTypeCount> cobFunctionCodes =
{
	0x000,	// DUMMY
	0x000,	// NMT
	0x080,	// SYNC
	0x080,	// EMCY
	0x100,	// TIME
	0x180,	// TPDO1
	0x200,	// RPDO1
	0x280,	// TPDO2
	0x300,	// RPDO2
	0x380,	// TPDO3
	0x400,	// RPDO3
	0x480,	// TPDO4
	0x500,	// RPDO4
	0x580,	// TSDO
	0x600,	// RSDO
	0x700	// HEARTBEAT
};


/**
 * @brief Calculates COB ID.
 *
 * @param cobType - COB type
 * @param nodeId - node ID
 * @return COB ID.
 */
inline uint32_t calculateCobId(CobType cobType, unsigned int nodeId)
{
	if ((cobType == CobType::Nmt)
			|| (cobType == CobType::Sync)
			|| (cobType == CobType::Time))
	{
		return cobFunctionCodes[cobType.underlying_value()];
	}
	return cobFunctionCodes[cobType.underlying_value()] + nodeId;
}


/// COB data length
const emb::Array<unsigned int, cobTypeCount> cobDataLen =
{
	0,	// DUMMY
	2,	// NMT
	0,	// SYNC
	2,	// EMCY
	6,	// TIME
	8,	// TPDO1
	8,	// RPDO1
	8,	// TPDO2
	8,	// RPDO2
	8,	// TPDO3
	8,	// RPDO3
	8,	// TPDO4
	8,	// RPDO4
	8,	// TSDO
	8,	// RSDO
	1	// HEARTBEAT
};


/// TPDO type
SCOPED_ENUM_DECLARE_BEGIN(TpdoType)
{
	Tpdo1,
	Tpdo2,
	Tpdo3,
	Tpdo4,
}
SCOPED_ENUM_DECLARE_END(TpdoType)


inline CobType toCobType(TpdoType tpdoType)
{
	return static_cast<CobType>(static_cast<unsigned int>(CobType::Tpdo1) + 2 * tpdoType.underlying_value());
}


/// RPDO type
SCOPED_ENUM_DECLARE_BEGIN(RpdoType)
{
	Rpdo1,
	Rpdo2,
	Rpdo3,
	Rpdo4,
}
SCOPED_ENUM_DECLARE_END(RpdoType)


inline CobType toCobType(RpdoType rpdoType)
{
	return static_cast<CobType>(
		static_cast<unsigned int>(CobType::Rpdo1) + 2 * static_cast<unsigned int>(rpdoType.underlying_value())
	);
}


/// SDO message data union.
union CobSdoData
{
	int32_t i32;
	uint32_t u32;
	float f32;
};


/**
 * @brief SDO message.
 *
 */
struct CobSdo
{
	uint32_t dataSizeIndicated : 1;
	uint32_t expeditedTransfer : 1;
	uint32_t dataEmptyBytes : 2;
	uint32_t reserved : 1;
	uint32_t cs : 3;
	uint32_t index : 16;
	uint32_t subindex : 8;
	CobSdoData data;
	CobSdo() { memset(this, 0, sizeof(CobSdo)); }
	CobSdo(uint64_t rawMsg) { memcpy(this, &rawMsg, sizeof(CobSdo)); }
	uint64_t all() const
	{
		uint64_t data = 0;
		memcpy(&data, this, sizeof(CobSdo));
		return data;
	}
};


// SDO cs-codes
namespace cs_codes {
const uint32_t sdoCcsWrite = 1;
const uint32_t sdoScsWrite = 3;
const uint32_t sdoCcsRead = 2;
const uint32_t sdoScsRead = 2;
}


/// OD access possible statuses
SCOPED_ENUM_DECLARE_BEGIN(ODAccessStatus)
{
	Success = 0,
	Fail = 1,
	NoAccess = 2
}
SCOPED_ENUM_DECLARE_END(ODAccessStatus)


/// OD entry data types
enum ODEntryDataType
{
	OD_BOOL,
	OD_INT16,
	OD_INT32,
	OD_UINT16,
	OD_UINT32,
	OD_FLOAT32,
	OD_ENUM16,
	OD_TASK,
	OD_STRING_4CHARS
};


/// OD entry access right types
enum ODEntryAccessRight
{
	OD_ACCESS_RW,
	OD_ACCESS_RO,
	OD_ACCESS_WO,
};


/// OD entry data types sizes
const size_t odEntryDataSizes[9] = {sizeof(bool), sizeof(int16_t), sizeof(int32_t),
		sizeof(uint16_t), sizeof(uint32_t), sizeof(float), sizeof(uint16_t), 0, 0};


/**
 * @brief OD entry key.
 *
 */
struct ODEntryKey
{
	uint32_t index;
	uint32_t subindex;
};


/**
 * @brief OD entry value.
 *
 */
struct ODEntryValue
{
	const char* category;
	const char* subcategory;
	const char* name;
	const char* unit;
	ODEntryDataType dataType;
	ODEntryAccessRight accessRight;
	uint32_t* dataPtr;
	ODAccessStatus (*readAccessFunc)(CobSdoData& dest);
	ODAccessStatus (*writeAccessFunc)(CobSdoData val);
};


/**
 * @brief OD entry.
 *
 */
struct ODEntry
{
	ODEntryKey key;
	ODEntryValue value;

	/**
	 * @brief Checks OD-entry read access.
	 *
	 * @param (none)
	 * @return \c true if entry has read access, \c false otherwise.
	 */
	bool hasReadAccess() const
	{
		return (value.accessRight == OD_ACCESS_RW) || (value.accessRight == OD_ACCESS_RO);
	}


	/**
	 * @brief Checks OD-entry write access.
	 *
	 * @param (none)
	 * @return \c true if entry has write access, \c false otherwise.
	 */
	bool hasWriteAccess() const
	{
		return (value.accessRight == OD_ACCESS_RW) || (value.accessRight == OD_ACCESS_WO);
	}
};


inline bool operator<(const ODEntry& lhs, const ODEntry& rhs)
{
	return (lhs.key.index < rhs.key.index)
			|| ((lhs.key.index == rhs.key.index) && (lhs.key.subindex < rhs.key.subindex));
}


/**
 * @brief OD entry key aux class. C++03 doesn't have direct {}-initialization.
 *
 */
struct ODEntryKeyAux
{
	uint32_t index;
	uint32_t subindex;
	ODEntryKeyAux(uint32_t _index, uint32_t _subindex) : index(_index), subindex(_subindex) {}
};


inline bool operator<(const ODEntryKeyAux& lhs, const ODEntry& rhs)
{
	return (lhs.index < rhs.key.index)
			|| ((lhs.index == rhs.key.index) && (lhs.subindex < rhs.key.subindex));
}


inline bool operator==(const ODEntryKeyAux& lhs, const ODEntry& rhs)
{
	return (lhs.index == rhs.key.index) && (lhs.subindex == rhs.key.subindex);
}


/// Used in OD-entries which doesn't have direct access to data through pointer.
#define OD_NO_DIRECT_ACCESS static_cast<uint32_t*>(NULL)


/// Used in OD-entries which have direct access to data through pointer.
#define OD_PTR(ptr) reinterpret_cast<uint32_t*>(ptr)


/// Used in OD-entries which don't have read access to data through function.
inline ODAccessStatus OD_NO_INDIRECT_READ_ACCESS(CobSdoData& dest) { return ODAccessStatus::NoAccess; }


/// Used in OD-entries which don't have write access to data through function.
inline ODAccessStatus OD_NO_INDIRECT_WRITE_ACCESS(CobSdoData val) { return ODAccessStatus::NoAccess; }


/// OD_TASK execution status
SCOPED_ENUM_DECLARE_BEGIN(TaskStatus)
{
	Success = 0,
	Fail = 1,
	InProgress = 2,
	Started = 3
}
SCOPED_ENUM_DECLARE_END(TaskStatus)


/// @}
} // namespace ucanopen


