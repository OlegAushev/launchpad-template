/**
 * @file
 * @ingroup mcu mcu_can
 */


#include "mcu_can.h"


namespace mcu {


namespace can {


const uint32_t impl::canBases[2] = {CANA_BASE, CANB_BASE};
const uint32_t impl::canPieIntNums[2] = {INT_CANA0, INT_CANB0};


} // namespace can


} // namespace mcu


