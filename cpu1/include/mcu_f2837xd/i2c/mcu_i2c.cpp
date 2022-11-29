/**
 * @file mcu_i2c.cpp
 * @ingroup mcu mcu_i2c
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "mcu_i2c.h"


namespace mcu {

namespace i2c {


const uint32_t impl::i2cBases[2] = {I2CA_BASE, I2CB_BASE};


} // namespace i2c


} // namespace mcu


