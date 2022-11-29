/**
 * @file emb_testrunner.h
 * @ingroup emb
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once


#include <stdint.h>
#include <stddef.h>
#include <cstdio>
#include <cstring>
#include <assert.h>


namespace emb {
/// @addtogroup emb
/// @{


void run_tests();


/**
 * @brief
 */
class TestRunner
{
public:
	static void (*print)(const char* str);
	static void (*print_nextline)();

private:
	static void print_dbg(const char* str) { printf(str); }
	static void print_nextline_dbg() { printf("\n"); }

private:
	static int s_nAssertsTotal;
	static int s_nAssertsFailedInTest;
	static int s_nAssertsFailedTotal;
	static int s_nTestsTotal;
	static int s_nTestsFailed;
	static int s_nTestsPassed;

public:
	template <typename T, typename U>
	static void assertEqual(const T& t, const U& u, const char* hint)
	{
		++s_nAssertsTotal;
		if (!(t == u))
		{
			++s_nAssertsFailedInTest;
			++s_nAssertsFailedTotal;
			print(hint);
			print_nextline();
		}
	}

	static void assertTrue(bool b, const char* hint)
	{
		assertEqual(b, true, hint);
	}


	template <class TestFunc>
	static void runTest(TestFunc testFunc, const char* test_name)
	{
		++s_nTestsTotal;
		s_nAssertsFailedInTest = 0;
		testFunc();
		if (s_nAssertsFailedInTest == 0)
		{
			++s_nTestsPassed;
			print("[ PASSED ] ");
			print(test_name);
			print_nextline();
		}
		else
		{
			++s_nTestsFailed;
			print("[ FAILED ] ");
			print(test_name);
			print_nextline();
		}
	}

	static void printResult()
	{
		print_nextline();

		char str[64] = {0};
		snprintf(str, 63, "Asserts: %d failed, %d passed", s_nAssertsFailedTotal, s_nAssertsTotal - s_nAssertsFailedTotal);
		print(str);
		print_nextline();

		memset(str, 0, 64);
		snprintf(str, 63, "Tests:   %d failed, %d passed", s_nTestsFailed, s_nTestsPassed);
		print(str);
		print_nextline();

		if (s_nTestsFailed == 0)
		{
			print("OK");
			print_nextline();
		}
		else
		{
			print("FAIL");
			print_nextline();
		}
	}

	static bool passed()
	{
		if (s_nTestsFailed == 0)
		{
			return true;
		}
		return false;
	}
};


/// @}
} // namespace emb


/// @addtogroup emb
/// @{


/**
 * @brief
 */
#define EMB_RUN_TEST(func) emb::TestRunner::runTest(func, #func)


/**
 * @brief
 */
#ifdef ON_TARGET_TEST_BUILD
#define EMB_ASSERT_EQUAL(x, y) \
{ \
	const char* hint = "[  WARN  ] Assertion failed: " #x " != " #y ", file: " __FILE__ ", line: " _STR(__LINE__); \
	emb::TestRunner::assertEqual(x, y, hint); \
}
#else
#define EMB_ASSERT_EQUAL(x, y) ((void)0)
#endif


/**
 * @brief
 */
#ifdef ON_TARGET_TEST_BUILD
#define EMB_ASSERT_TRUE(x) \
{ \
	const char* hint = "[  WARN  ] Assertion failed: " #x " is false, file: " __FILE__ ", line: " _STR(__LINE__); \
	emb::TestRunner::assertTrue(x, hint); \
}
#else
#define EMB_ASSERT_TRUE(x) ((void)0)
#endif


/// @}


