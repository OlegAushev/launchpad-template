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
#include "mcu_f2837xd/chrono/mcu_chrono.h"
#include "mcu_f2837xd/spi/mcu_spi.h"
#include "emb/emb_pair.h"
#include "emb/emb_math.h"
#include "bsp_launchxl_f28379d/bsp_launchxl_f28379d.h"


/**
 * @brief Toggling led task.
 * @return Task execution status.
 */
mcu::chrono::TaskStatus taskToggleLed(size_t taskIndex);


