/**
 * @file mcu_qep.cpp
 * @ingroup mcu mcu_qep
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "mcu_qep.h"


namespace mcu {


namespace qep {


const uint32_t impl::qepBases[3] = {EQEP1_BASE, EQEP2_BASE, EQEP3_BASE};
const uint32_t impl::qepPieIntNums[3] = {INT_EQEP1, INT_EQEP2, INT_EQEP3};


} // namespace qep


} // namespace mcu


