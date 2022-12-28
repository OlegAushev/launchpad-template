/**
 * @file emb_testrunner.cpp
 * @ingroup emb
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "emb_testrunner.h"


namespace emb {
/// @addtogroup emb
/// @{


void (*TestRunner::print)(const char* str) = TestRunner::print_dbg;
void (*TestRunner::print_nextline)() = TestRunner::print_nextline_dbg;

int TestRunner::_nAssertsTotal = 0;
int TestRunner::_nAssertsFailedInTest = 0;
int TestRunner::_nAssertsFailedTotal = 0;
int TestRunner::_nTestsTotal = 0;
int TestRunner::_nTestsFailed = 0;
int TestRunner::_nTestsPassed = 0;


/// @}
}


