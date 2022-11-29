/**
 * @file mcu_spi.cpp
 * @ingroup mcu mcu_spi
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "mcu_spi.h"


namespace mcu {


namespace spi {


const uint32_t impl::spiBases[3] = {SPIA_BASE, SPIB_BASE, SPIC_BASE};
const uint32_t impl::spiRxPieIntNums[3] = {INT_SPIA_RX, INT_SPIB_RX, INT_SPIC_RX};


} // namespace spi


} // namespace mcu


