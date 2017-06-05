#include <gtest/gtest.h>
#include "CTestDll.h"
#include "CHsmProxy.h"
#include "testConf.h"

TEST(DllTest, simple)
{
	CTestDll dll;

	CHsmProxy p;
	ASSERT_EQ(p.init(SO_PIN, USER_PIN), 0);
}