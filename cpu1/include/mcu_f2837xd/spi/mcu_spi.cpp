/**
 * @file
 * @ingroup mcu mcu_spi
 */


#include "mcu_spi.h"


namespace mcu {


namespace spi {


const uint32_t impl::spiBases[3] = {SPIA_BASE, SPIB_BASE, SPIC_BASE};
const uint32_t impl::spiRxPieIntNums[3] = {INT_SPIA_RX, INT_SPIB_RX, INT_SPIC_RX};


} // namespace spi


} // namespace mcu


