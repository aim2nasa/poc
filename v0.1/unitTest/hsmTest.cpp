#include <gtest/gtest.h>
#include "CHsmProxy.h"
#include "common.h"
#include "testConf.h"

TEST(HsmTest, simple)
{
	EXPECT_NE(1, 2);
	CHsmProxy p;
	ASSERT_EQ(p.init(USER_PIN), 0);

	//���� sizeof�� �ϸ� NULL���� �����ؼ� ũ�Ⱑ ���´� ���� -1�� ����� ��
	unsigned long hTagKey, hSeKey;
	hTagKey=hSeKey=0;	//CK_INVALID_HANDLE = 0

	EXPECT_EQ(p.findKey(TAG_KEY_LABEL, sizeof(TAG_KEY_LABEL)-1,hTagKey),0);
	EXPECT_NE(hTagKey, 0);
	EXPECT_EQ(p.findKey(SE_KEY_LABEL, sizeof(SE_KEY_LABEL)-1, hSeKey), 0);
	EXPECT_NE(hSeKey, 0);

	//������ ������ ��ü(Ű)�� �̿��ؼ� ��ȣȭ �� ��ȣȭ�� �����Ѵ�. (AES ECB encoding/decoding test)
	const int blockSize(0x10);
	const int NumBlock(10);
	char data[blockSize*NumBlock] = { "Billy Elliot the musical. What a fascinating experince! Love it" };

	//AES ECB encoding
	EXPECT_EQ(p.encryptInit(CHsmProxy::AES_ECB,hTagKey),0);

	unsigned long ulEncryptedDataLen;
	EXPECT_EQ(p.encrypt((unsigned char*)data, sizeof(data), NULL, &ulEncryptedDataLen), 0);

	std::vector<unsigned char> vEncryptedData;
	vEncryptedData.resize(ulEncryptedDataLen);
	EXPECT_EQ(p.encrypt((unsigned char*)data, sizeof(data), &vEncryptedData.front(), &ulEncryptedDataLen), 0);

	//AES ECB decoding
	EXPECT_EQ(p.decryptInit(CHsmProxy::AES_ECB, hTagKey), 0);

	unsigned long ulDataLen;
	EXPECT_EQ(p.decrypt(&vEncryptedData.front(), (unsigned long)vEncryptedData.size(), NULL, &ulDataLen), 0);

	std::vector<unsigned char> vDecryptedData;
	vDecryptedData.resize(ulDataLen);
	EXPECT_EQ(p.decrypt(&vEncryptedData.front(), (unsigned long)vEncryptedData.size(), &vDecryptedData.front(), &ulDataLen), 0);

	//���ڵ��� ����� ������ �����Ϳ� ���ƾ� �Ѵ�.
	EXPECT_EQ(sizeof(data), ulDataLen);
	EXPECT_EQ(memcmp(data, &vDecryptedData.front(), sizeof(data)), 0);
}

TEST(HsmTest, twoTokens)
{
	EXPECT_NE(1, 2);
	CHsmProxy p1;
	ASSERT_EQ(p1.init(USER_PIN), 0);			//���ǻ� p1�� p2�� ������ ������ SO pin,UserPin���� ���õǾ� ����

	CHsmProxy p2;
	//softhsm2���̺귯���� singleton���� ��ü�� �����Ͽ� ���μ����� �Ѱ� �̻��� �ν��Ͻ��� ������ �ʴ´�.
	//���� �Ʒ��� ���� �ٽ� ���̺귯���� �ʱ�ȭ�� �Ϸ��� �ϸ� �̹� �ʱ�ȭ �Ǿ��ٰ� ������ �Ѵ�.
	//�ش� ���μ��� ���������� �ν��Ͻ��� ���̻� ����� �����Ƿ� �Ʒ��� �ʱ�ȭ�� �����Ѵ�.
	ASSERT_EQ(p2.init(USER_PIN), -1);
}

TEST(HsmTest, env)
{
	CHsmProxy p1;
	p1.setenv("SOFTHSM2_CONF", ".\\softhsm2.conf", 1);
	ASSERT_EQ(p1.init(USER_PIN), 0);
}