///
#pragma once


#include <stdint.h>
#include <stddef.h>
#include <cstdio>
#include <cstring>
#include <assert.h>


namespace emb {


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
	static int nAssertsTotal;
	static int nAssertsFailedInTest;
	static int nAssertsFailedTotal;
	static int nTestsTotal;
	static int nTestsFailed;
	static int nTestsPassed;

public:
	template <typename T, typename U>
	static void assertEqual(const T& t, const U& u, const char* hint)
	{
		++nAssertsTotal;
		if (!(t == u))
		{
			++nAssertsFailedInTest;
			++nAssertsFailedTotal;
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
		++nTestsTotal;
		nAssertsFailedInTest = 0;
		testFunc();
		if (nAssertsFailedInTest == 0)
		{
			++nTestsPassed;
			print("[ PASSED ] ");
			print(test_name);
			print_nextline();
		}
		else
		{
			++nTestsFailed;
			print("[ FAILED ] ");
			print(test_name);
			print_nextline();
		}
	}

	static void printResult()
	{
		print_nextline();

		char str[64] = {0};
		snprintf(str, 63, "Asserts: %d failed, %d passed", nAssertsFailedTotal, nAssertsTotal - nAssertsFailedTotal);
		print(str);
		print_nextline();

		memset(str, 0, 64);
		snprintf(str, 63, "Tests:   %d failed, %d passed", nTestsFailed, nTestsPassed);
		print(str);
		print_nextline();

		if (nTestsFailed == 0)
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
		if (nTestsFailed == 0)
		{
			return true;
		}
		return false;
	}
};


} // namespace emb


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


