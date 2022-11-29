/**
 * @file mcu_cap.cpp
 * @ingroup mcu mcu_cap
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "mcu_cap.h"


namespace mcu {


namespace impl {


const uint32_t capBases[6] = {ECAP1_BASE, ECAP2_BASE, ECAP3_BASE, ECAP4_BASE, ECAP5_BASE, ECAP6_BASE};
const XBAR_InputNum capXbarInputs[6] = {XBAR_INPUT7, XBAR_INPUT8, XBAR_INPUT9, XBAR_INPUT10, XBAR_INPUT11, XBAR_INPUT12};
const uint32_t capPieIntNums[6] = {INT_ECAP1, INT_ECAP2, INT_ECAP3, INT_ECAP4, INT_ECAP5,INT_ECAP6};


}


} // namespace mcu


