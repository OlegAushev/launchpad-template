/**
 * @file mcu_pwm.cpp
 * @ingroup mcu mcu_pwm
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "mcu_pwm.h"


namespace mcu {


namespace pwm {


const uint32_t impl::pwmBases[12] = {EPWM1_BASE, EPWM2_BASE, EPWM3_BASE, EPWM4_BASE,
					EPWM5_BASE, EPWM6_BASE, EPWM7_BASE, EPWM8_BASE,
					EPWM9_BASE, EPWM10_BASE, EPWM11_BASE, EPWM12_BASE};
const uint32_t impl::pwmPieEventIntNums[12] = {INT_EPWM1, INT_EPWM2, INT_EPWM3, INT_EPWM4,
						INT_EPWM5, INT_EPWM6, INT_EPWM7, INT_EPWM8,
						INT_EPWM9, INT_EPWM10, INT_EPWM11, INT_EPWM12};
const uint32_t impl::pwmPieTripIntNums[12] = {INT_EPWM1_TZ, INT_EPWM2_TZ, INT_EPWM3_TZ, INT_EPWM4_TZ,
						INT_EPWM5_TZ, INT_EPWM6_TZ, INT_EPWM7_TZ, INT_EPWM8_TZ,
						INT_EPWM9_TZ, INT_EPWM10_TZ, INT_EPWM11_TZ, INT_EPWM12_TZ};

const uint32_t impl::pwmPinOutAConfigs[12] = {GPIO_0_EPWM1A, GPIO_2_EPWM2A, GPIO_4_EPWM3A, GPIO_6_EPWM4A,
						GPIO_8_EPWM5A, GPIO_10_EPWM6A, GPIO_12_EPWM7A, GPIO_14_EPWM8A,
						GPIO_16_EPWM9A, GPIO_18_EPWM10A, GPIO_20_EPWM11A, GPIO_22_EPWM12A};
const uint32_t impl::pwmPinOutBConfigs[12] = {GPIO_1_EPWM1B, GPIO_3_EPWM2B, GPIO_5_EPWM3B, GPIO_7_EPWM4B,
						GPIO_9_EPWM5B, GPIO_11_EPWM6B, GPIO_13_EPWM7B, GPIO_15_EPWM8B,
						GPIO_17_EPWM9B, GPIO_19_EPWM10B, GPIO_21_EPWM11B, GPIO_23_EPWM12B};


} // namespace pwm


} // namespace mcu


