#include <gtest/gtest.h>
#include "CHsmProxy.h"
#include "protocol.h"

TEST(HsmTest, simple)
{
	EXPECT_NE(1, 2);
	CHsmProxy p;
	EXPECT_EQ(p.init("123456", "1234"),0);

	//주의 sizeof를 하면 NULL까지 포함해서 크기가 나온다 따라서 -1을 해줘야 함
	unsigned long hTagKey, hSeKey;
	hTagKey=hSeKey=0;	//CK_INVALID_HANDLE = 0

	EXPECT_EQ(p.findKey(TAG_KEY_LABEL, sizeof(TAG_KEY_LABEL)-1,hTagKey),0);
	EXPECT_NE(hTagKey, 0);
	EXPECT_EQ(p.findKey(SE_KEY_LABEL, sizeof(SE_KEY_LABEL)-1, hSeKey), 0);
	EXPECT_NE(hSeKey, 0);
}