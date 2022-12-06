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
emb::Queue<sys::Message::Message, 32> SysLog::s_messages __attribute__((section("shared_syslog_messages"), retain));
#else
emb::Queue<sys::Message::Message, 32> SysLog::s_messages;
#endif


#ifdef DUALCORE
sys::Message::Message SysLog::s_cpu2Message __attribute__((section("shared_syslog_message_cpu2"), retain)) = sys::Message::NoMessage;
#endif


#ifdef DUALCORE
SysLog::Data SysLog::s_cpu1Data __attribute__((section("shared_syslog_data_cpu1"), retain));
SysLog::Data SysLog::s_cpu2Data __attribute__((section("shared_syslog_data_cpu2"), retain));
#else
SysLog::Data SysLog::s_cpu1Data;
#endif


SysLog::Data* SysLog::s_thisCpuData;


// IPC flags
SysLog::IpcFlags SysLog::s_ipcFlags;


