/**
 * @file mcu_gpio.cpp
 * @ingroup mcu mcu_gpio
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "mcu_gpio.h"


namespace mcu {


namespace gpio {


const uint32_t impl::pieXIntNums[5] = {INT_XINT1, INT_XINT2, INT_XINT3, INT_XINT4, INT_XINT5};
const uint16_t impl::pieXIntGroups[5] = {INTERRUPT_ACK_GROUP1, INTERRUPT_ACK_GROUP1,
		INTERRUPT_ACK_GROUP12, INTERRUPT_ACK_GROUP12, INTERRUPT_ACK_GROUP12};


} // namespace gpio


} // namespace mcu


