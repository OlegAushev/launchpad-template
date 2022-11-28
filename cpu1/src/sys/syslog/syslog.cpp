/**
 * @file
 * @ingroup syslog
 */


#include "syslog.h"


#ifdef DUALCORE
emb::Queue<sys::Message::Message, 32> SysLog::m_messages __attribute__((section("SHARED_SYSLOG_MESSAGES"), retain));
#else
emb::Queue<sys::Message::Message, 32> SysLog::m_messages;
#endif


#ifdef DUALCORE
sys::Message::Message SysLog::m_cpu2Message __attribute__((section("SHARED_SYSLOG_MESSAGE_CPU2"), retain)) = sys::Message::NO_MESSAGE;
#endif


#ifdef DUALCORE
SysLog::Data SysLog::m_cpu1Data __attribute__((section("SHARED_SYSLOG_DATA_CPU1"), retain));
SysLog::Data SysLog::m_cpu2Data __attribute__((section("SHARED_SYSLOG_DATA_CPU2"), retain));
#else
SysLog::Data SysLog::m_cpu1Data;
#endif


SysLog::Data* SysLog::m_thisCpuData;


// IPC flags
SysLog::IpcFlags SysLog::s_ipcFlags;


