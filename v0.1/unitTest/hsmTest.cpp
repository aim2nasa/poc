#include <gtest/gtest.h>
#include "CHsmProxy.h"
#include "common.h"

TEST(HsmTest, simple)
{
	EXPECT_NE(1, 2);
	CHsmProxy p;
	ASSERT_EQ(p.init("123456", "1234"), 0);

	//주의 sizeof를 하면 NULL까지 포함해서 크기가 나온다 따라서 -1을 해줘야 함
	unsigned long hTagKey, hSeKey;
	hTagKey=hSeKey=0;	//CK_INVALID_HANDLE = 0

	EXPECT_EQ(p.findKey(TAG_KEY_LABEL, sizeof(TAG_KEY_LABEL)-1,hTagKey),0);
	EXPECT_NE(hTagKey, 0);
	EXPECT_EQ(p.findKey(SE_KEY_LABEL, sizeof(SE_KEY_LABEL)-1, hSeKey), 0);
	EXPECT_NE(hSeKey, 0);

	//위에서 생성된 객체(키)를 이용해서 암호화 및 복호화를 수행한다. (AES ECB encoding/decoding test)
	const int blockSize(0x10);
	const int NumBlock(10);
	char data[blockSize*NumBlock] = { "Billy Elliot the musical. What a fascinating experince! Love it" };

	//AES ECB encoding
	EXPECT_EQ(p.encryptInit(CHsmProxy::AES_ECB,hTagKey),0);
}