///
#include "emb_testrunner.h"


namespace emb {


void (*TestRunner::print)(const char* str) = TestRunner::print_dbg;
void (*TestRunner::print_nextline)() = TestRunner::print_nextline_dbg;

int TestRunner::nAssertsTotal = 0;
int TestRunner::nAssertsFailedInTest = 0;
int TestRunner::nAssertsFailedTotal = 0;
int TestRunner::nTestsTotal = 0;
int TestRunner::nTestsFailed = 0;
int TestRunner::nTestsPassed = 0;


}


