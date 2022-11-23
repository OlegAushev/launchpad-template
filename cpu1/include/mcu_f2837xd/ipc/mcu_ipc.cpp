/**
 * @file
 * @ingroup mcu mcu_ipc
 */


#include "mcu_ipc.h"


namespace mcu {


namespace ipc {


namespace flags {

#ifdef DUALCORE
const mcu::ipc::IpcMode mode = mcu::ipc::IpcModeSinglecore;
#else
const mcu::ipc::IpcMode mode = mcu::ipc::IpcModeDualcore;
#endif

mcu::ipc::Flag cpu1PeripheryConfigured(31, mode);
mcu::ipc::Flag cpu2Booted(30, mode);
mcu::ipc::Flag cpu2PeripheryConfigured(29, mode);

}


}


}


