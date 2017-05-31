#include <gtest/gtest.h>
#include "CHsmProxy.h"

TEST(HsmTest, simple)
{
	EXPECT_NE(1, 2);
	CHsmProxy p;
	EXPECT_EQ(p.init("123456", "1234"),0);
}