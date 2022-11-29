/**
 * @file syslog.cpp
 * @ingroup syslog
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "syslog.h"


#ifdef DUALCORE
emb::Queue<sys::Message::Message, 32> SysLog::s_messages __attribute__((section("SHARED_SYSLOG_MESSAGES"), retain));
#else
emb::Queue<sys::Message::Message, 32> SysLog::s_messages;
#endif


#ifdef DUALCORE
sys::Message::Message SysLog::s_cpu2Message __attribute__((section("SHARED_SYSLOG_MESSAGE_CPU2"), retain)) = sys::Message::NoMessage;
#endif


#ifdef DUALCORE
SysLog::Data SysLog::s_cpu1Data __attribute__((section("SHARED_SYSLOG_DATA_CPU1"), retain));
SysLog::Data SysLog::s_cpu2Data __attribute__((section("SHARED_SYSLOG_DATA_CPU2"), retain));
#else
SysLog::Data SysLog::s_cpu1Data;
#endif


SysLog::Data* SysLog::s_thisCpuData;


// IPC flags
SysLog::IpcFlags SysLog::s_ipcFlags;


