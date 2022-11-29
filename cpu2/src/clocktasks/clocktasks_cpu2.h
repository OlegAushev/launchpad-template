/**
 * @file clocktasks_cpu2.h
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once


#include "mcu_f2837xd/system/mcu_system.h"
#include "mcu_f2837xd/cputimers/mcu_cputimers.h"
#include "mcu_f2837xd/spi/mcu_spi.h"
#include "mcu_f2837xd/support/mcu_support.h"
#include "emb/emb_pair.h"
#include "emb/emb_math.h"


/**
 * @brief Toggling led task.
 * @return Task execution status.
 */
mcu::ClockTaskStatus taskToggleLed();


