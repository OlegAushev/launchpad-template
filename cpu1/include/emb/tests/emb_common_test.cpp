///
#include "emb_test.h"


class SingletonTest : public emb::c28x::singleton<SingletonTest>
{
public:
	SingletonTest() : singleton(this) {}
};


void EmbTest::CommonTest()
{
	SingletonTest singletonObject;
	SingletonTest* pSingletonObject = &singletonObject;
	EMB_ASSERT_EQUAL(SingletonTest::instance(), pSingletonObject);
}


