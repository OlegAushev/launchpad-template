/**
 * @file mcu_dac.cpp
 * @ingroup mcu mcu_dac
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "mcu_dac.h"


namespace mcu {


namespace dac {


const uint32_t impl::dacBases[3] = {DACA_BASE, DACB_BASE, DACC_BASE};


} // namespace dac


} // namespace mcu


