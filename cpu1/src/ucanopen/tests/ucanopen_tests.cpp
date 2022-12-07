/**
 * @file ucanopen_tests.cpp
 * @ingroup ucanopen
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief
 * @version 0.1
 * @date 2022-12-07
 *
 * @copyright Copyright (c) 2022
 *
 */


#include "ucanopen_tests.h"


namespace ucanopen {


namespace tests {


namespace od {


inline ODAccessStatus getDeviceName(CobSdoData& dest)
{
	char name[4] = {0};
	strncpy(name, SysInfo::deviceNameShort, 4);
	uint32_t nameRaw = 0;
	emb::c28x::from_bytes<uint32_t>(nameRaw, reinterpret_cast<uint16_t*>(name));
	memcpy(&dest, &nameRaw, sizeof(uint32_t));
	return ODAccessStatus::Success;
}


inline ODAccessStatus getFirmwareVersion(CobSdoData& dest)
{
	char name[4] = {0};
	strncpy(name, GIT_HASH, 4);
	uint32_t nameRaw = 0;
	emb::c28x::from_bytes<uint32_t>(nameRaw, reinterpret_cast<uint16_t*>(name));
	memcpy(&dest, &nameRaw, sizeof(uint32_t));
	return ODAccessStatus::Success;
}


inline ODAccessStatus getBuildConfiguration(CobSdoData& dest)
{
	char name[4] = {0};
	strncpy(name, SysInfo::buildConfigurationShort, 4);
	uint32_t nameRaw = 0;
	emb::c28x::from_bytes<uint32_t>(nameRaw, reinterpret_cast<uint16_t*>(name));
	memcpy(&dest, &nameRaw, sizeof(uint32_t));
	return ODAccessStatus::Success;
}


inline ODAccessStatus getSyslogMessage(CobSdoData& dest)
{
	uint32_t message = SysLog::readMessage().underlying_value();
	SysLog::popMessage();
	memcpy(&dest, &message, sizeof(uint32_t));
	return ODAccessStatus::Success;
}

inline ODAccessStatus getUptime(CobSdoData& dest)
{
	float time = mcu::chrono::SystemClock::now() / 1000.f;
	memcpy(&dest, &time, sizeof(uint32_t));
	return ODAccessStatus::Success;
}


} // namespace od


ODEntry objectDictionary[] = {
{{0x1008, 0x00}, {"SYSTEM", "INFO", "DEVICE_NAME", "", OD_STRING_4CHARS, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::getDeviceName, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x5FFF, 0x00}, {"SYSTEM", "INFO", "FIRMWARE_VERSION", "", OD_STRING_4CHARS, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::getFirmwareVersion, OD_NO_INDIRECT_WRITE_ACCESS}},
{{0x5FFF, 0x01}, {"SYSTEM", "INFO", "BUILD_CONFIGURATION", "", OD_STRING_4CHARS, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::getBuildConfiguration, OD_NO_INDIRECT_WRITE_ACCESS}},

{{0x2000, 0x00}, {"SYSTEM", "SYSLOG", "SYSLOG_MSG", "", OD_UINT32, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::getSyslogMessage, OD_NO_INDIRECT_WRITE_ACCESS}},

{{0x5000, 0x00}, {"WATCH", "WATCH", "UPTIME", "s", OD_FLOAT32, OD_ACCESS_RO, OD_NO_DIRECT_ACCESS, od::getUptime, OD_NO_INDIRECT_WRITE_ACCESS}},

};


const size_t objectDictionaryLen = sizeof(objectDictionary) / sizeof(objectDictionary[0]);


}


}


