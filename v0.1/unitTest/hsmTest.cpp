#include <gtest/gtest.h>
#include "CHsmProxy.h"
#include "common.h"
#include "testConf.h"

TEST(HsmTest, simple)
{
	EXPECT_NE(1, 2);
	CHsmProxy p;
	ASSERT_EQ(p.init(USER_PIN), 0);

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

	//디코딩된 결과는 원래의 데이터와 같아야 한다.
	EXPECT_EQ(sizeof(data), ulDataLen);
	EXPECT_EQ(memcmp(data, &vDecryptedData.front(), sizeof(data)), 0);
}

TEST(HsmTest, twoTokens)
{
	EXPECT_NE(1, 2);
	CHsmProxy p1;
	ASSERT_EQ(p1.init(USER_PIN), 0);			//편의상 p1과 p2는 사전에 동일한 SO pin,UserPin으로 세팅되어 있음

	CHsmProxy p2;
	//softhsm2라이브러리는 singleton으로 객체를 생성하여 프로세스당 한개 이상의 인스턴스가 생기지 않는다.
	//따라서 아래와 같이 다시 라이브러리를 초기화를 하려고 하면 이미 초기화 되었다고 응답을 한다.
	//해당 프로세스 공간에서는 인스턴스를 더이상 만들수 없으므로 아래의 초기화는 실패한다.
	ASSERT_EQ(p2.init(USER_PIN), -1);
}

TEST(HsmTest, env)
{
	CHsmProxy p1;
	p1.setenv("SOFTHSM2_CONF", ".\\softhsm2.conf", 1);
	ASSERT_EQ(p1.init(USER_PIN), 0);
}