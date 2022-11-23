/**
 * @file
 */


#pragma once


#include "F28x_Project.h"
#include "F2837xD_Ipc_drivers.h"
#include "mcu_f2837xd/system/mcu_system.h"
#include "mcu_f2837xd/support/mcu_support.h"
#include "mcu_f2837xd/cputimers/mcu_cputimers.h"
#include "sys/syslog/syslog.h"


/**
 * @brief Task performed at watchdog timeout.
 * @param (none)
 * @return Task execution status.
 */
mcu::ClockTaskStatus taskWatchdogTimeout();


/**
 * @brief Toggling led task.
 * @param (none)
 * @return Task execution status.
 */
mcu::ClockTaskStatus taskToggleLed(size_t taskIndex);


/**
 * @brief Start temperature sensors task.
 * @param (none)
 * @return Task execution status.
 */
mcu::ClockTaskStatus taskStartTempSensorAdc(size_t taskIndex);


