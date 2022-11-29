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

int TestRunner::s_nAssertsTotal = 0;
int TestRunner::s_nAssertsFailedInTest = 0;
int TestRunner::s_nAssertsFailedTotal = 0;
int TestRunner::s_nTestsTotal = 0;
int TestRunner::s_nTestsFailed = 0;
int TestRunner::s_nTestsPassed = 0;


/// @}
}


