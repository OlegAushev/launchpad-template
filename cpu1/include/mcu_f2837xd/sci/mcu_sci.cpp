/**
 * @file
 * @ingroup mcu mcu_sci
 */


#include "mcu_sci.h"


namespace mcu {


namespace sci {


const uint32_t impl::sciBases[4] = {SCIA_BASE, SCIB_BASE, SCIC_BASE, SCID_BASE};
const uint32_t impl::sciRxPieIntNums[4] = {INT_SCIA_RX, INT_SCIB_RX, INT_SCIC_RX, INT_SCID_RX};
const uint16_t impl::sciPieIntGroups[4] = {INTERRUPT_ACK_GROUP9, INTERRUPT_ACK_GROUP9,
						INTERRUPT_ACK_GROUP8, INTERRUPT_ACK_GROUP8};


} // namespace sci


} // namespace mcu


