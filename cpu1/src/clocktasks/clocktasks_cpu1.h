/**
 * @file clocktasks_cpu1.h
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once


#include "F28x_Project.h"
#include "F2837xD_Ipc_drivers.h"
#include "mcu_f2837xd/system/mcu_system.h"
#include "mcu_f2837xd/chrono/mcu_chrono.h"
#include "bsp_launchxl_f28379d/leds/leds.h"
#include "sys/syslog/syslog.h"


/**
 * @brief Task performed at watchdog timeout.
 * @param (none)
 * @return Task execution status.
 */
mcu::chrono::TaskStatus taskWatchdogTimeout();


/**
 * @brief Toggling led task.
 * @param (none)
 * @return Task execution status.
 */
mcu::chrono::TaskStatus taskToggleLed(size_t taskIndex);


/**
 * @brief Start temperature sensors task.
 * @param (none)
 * @return Task execution status.
 */
mcu::chrono::TaskStatus taskStartTempSensorAdc(size_t taskIndex);


