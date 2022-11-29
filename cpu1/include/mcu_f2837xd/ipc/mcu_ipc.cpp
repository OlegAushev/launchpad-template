/**
 * @file mcu_ipc.cpp
 * @ingroup mcu mcu_ipc
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "mcu_ipc.h"


namespace mcu {


namespace ipc {


namespace flags {

#ifdef DUALCORE
const mcu::ipc::IpcMode mode = mcu::ipc::IpcMode::Singlecore;
#else
const mcu::ipc::IpcMode mode = mcu::ipc::IpcMode::Dualcore;
#endif

mcu::ipc::Flag cpu1PeripheryConfigured(31, mode);
mcu::ipc::Flag cpu2Booted(30, mode);
mcu::ipc::Flag cpu2PeripheryConfigured(29, mode);

}


}


}


