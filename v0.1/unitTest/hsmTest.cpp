#include <gtest/gtest.h>
#include "CHsmProxy.h"
#include "protocol.h"

TEST(HsmTest, simple)
{
	EXPECT_NE(1, 2);
	CHsmProxy p;
	EXPECT_EQ(p.init("123456", "1234"),0);

	//���� sizeof�� �ϸ� NULL���� �����ؼ� ũ�Ⱑ ���´� ���� -1�� ����� ��
	unsigned long hTagKey, hSeKey;
	hTagKey=hSeKey=0;	//CK_INVALID_HANDLE = 0

	EXPECT_EQ(p.findKey(TAG_KEY_LABEL, sizeof(TAG_KEY_LABEL)-1,hTagKey),0);
	EXPECT_NE(hTagKey, 0);
	EXPECT_EQ(p.findKey(SE_KEY_LABEL, sizeof(SE_KEY_LABEL)-1, hSeKey), 0);
	EXPECT_NE(hSeKey, 0);
}