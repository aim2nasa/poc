#include <gtest/gtest.h>
#include "CHsmProxy.h"
#include "protocol.h"
#include "pkcs11.h"
#include "CToken.h"

TEST(HsmTest, simple)
{
	EXPECT_NE(1, 2);
	CHsmProxy p;
	ASSERT_EQ(p.init("123456", "1234"), 0);

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
	CK_BYTE data[blockSize*NumBlock] = { "Billy Elliot the musical. What a fascinating experince! Love it" };

	//AES ECB encoding
	const CK_MECHANISM mechanismEnc = { CKM_AES_ECB, NULL_PTR, 0 };
	CK_MECHANISM_PTR pMechanism((CK_MECHANISM_PTR)&mechanismEnc);

	EXPECT_EQ(C_EncryptInit(p.token().session(), pMechanism, hTagKey), CKR_OK);

	CK_ULONG ulEncryptedDataLen;
	EXPECT_EQ(C_Encrypt(p.token().session(), data, sizeof(data), NULL_PTR, &ulEncryptedDataLen), CKR_OK);

	std::vector<CK_BYTE> vEncryptedData;
	vEncryptedData.resize(ulEncryptedDataLen);
	EXPECT_EQ(C_Encrypt(p.token().session(), data, sizeof(data), &vEncryptedData.front(), &ulEncryptedDataLen), CKR_OK);

	//AES ECB decoding
	EXPECT_EQ(C_DecryptInit(p.token().session(), pMechanism, hTagKey), CKR_OK);

	CK_ULONG ulDataLen;
	EXPECT_EQ(C_Decrypt(p.token().session(), &vEncryptedData.front(), (unsigned long)vEncryptedData.size(), NULL_PTR, &ulDataLen), CKR_OK);

	std::vector<CK_BYTE> vDecryptedData;
	vDecryptedData.resize(ulDataLen);
	EXPECT_EQ(C_Decrypt(p.token().session(), &vEncryptedData.front(), (unsigned long)vEncryptedData.size(), &vDecryptedData.front(), &ulDataLen), CKR_OK);

	//���ڵ��� ����� ������ �����Ϳ� ���ƾ� �Ѵ�.
	EXPECT_EQ(sizeof(data), ulDataLen);
	EXPECT_EQ(memcmp(data, &vDecryptedData.front(), sizeof(data)), 0);
}