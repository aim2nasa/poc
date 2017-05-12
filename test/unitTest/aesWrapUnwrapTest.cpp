#include "HsmTest.h"
#include "..\keyGen\helper.h"

const CK_BBOOL ON_TOKEN = CK_TRUE;
const CK_BBOOL IN_SESSION = CK_FALSE;
const CK_BBOOL IS_PRIVATE = CK_TRUE;
const CK_BBOOL IS_PUBLIC = CK_FALSE;

class AesWrapUnwrapTest : public HsmTest{
protected:
	AesWrapUnwrapTest() :_hSession(CK_INVALID_HANDLE){}

	virtual void SetUp()
	{
		HsmTest::SetUp();
		EXPECT_NE(this->_p11, reinterpret_cast<CK_FUNCTION_LIST_PTR>(NULL));
		EXPECT_EQ(prepare(".\\softhsm2.conf", this->_p11, "1234", "MyToken", "1234", _hSession), 0);
	}

	virtual void TearDown()
	{
		HsmTest::TearDown();
	}

	CK_RV generateAesKey(CK_BBOOL bToken, CK_BBOOL bPrivate, CK_ULONG bytes, const char *gw, CK_OBJECT_HANDLE &hKey);
	void generateAesKeyTest(CK_BBOOL bToken, CK_BBOOL bPrivate, CK_ULONG bytes, const char *gw, CK_OBJECT_HANDLE &hKey);

	CK_SESSION_HANDLE _hSession;
};

CK_RV AesWrapUnwrapTest::generateAesKey(CK_BBOOL bToken, CK_BBOOL bPrivate, CK_ULONG keySize, const char *gw, CK_OBJECT_HANDLE &hKey)
{
	CK_MECHANISM mechanism = { CKM_AES_KEY_GEN, NULL_PTR, 0 };
	CK_BBOOL bTrue = CK_TRUE;
	CK_BBOOL bFalse = CK_FALSE;
	CK_ATTRIBUTE keyAttribs[] = {
		{ CKA_TOKEN, &bToken, sizeof(bToken) },
		{ CKA_PRIVATE, &bPrivate, sizeof(bPrivate) },
		{ CKA_EXTRACTABLE, &bTrue, sizeof(bTrue) },
		{ CKA_SENSITIVE, &bFalse, sizeof(bTrue) },
		{ CKA_DERIVE, &bTrue, sizeof(bTrue) },
		{ CKA_ENCRYPT, &bTrue, sizeof(bTrue) },
		{ CKA_DECRYPT, &bTrue, sizeof(bTrue) },
		{ CKA_WRAP, &bTrue, sizeof(bTrue) },
		{ CKA_UNWRAP, &bTrue, sizeof(bTrue) },
		{ CKA_VALUE_LEN, &keySize, sizeof(keySize) }
	};

	hKey = CK_INVALID_HANDLE;
	return C_GenerateKey(_hSession, &mechanism, keyAttribs, sizeof(keyAttribs) / sizeof(CK_ATTRIBUTE), &hKey);
}

void AesWrapUnwrapTest::generateAesKeyTest(CK_BBOOL bToken, CK_BBOOL bPrivate, CK_ULONG keySize, const char *gw, CK_OBJECT_HANDLE &hKey)
{
	EXPECT_NE(_hSession, CK_INVALID_HANDLE);
	hKey = CK_INVALID_HANDLE;
	EXPECT_EQ(generateAesKey(bToken, bPrivate, keySize, gw, hKey), CKR_OK);
	EXPECT_NE(hKey, CK_INVALID_HANDLE);
}

TEST_F(AesWrapUnwrapTest, sessionopen)
{
	EXPECT_NE(_hSession, CK_INVALID_HANDLE);
}

TEST_F(AesWrapUnwrapTest, aesKeyGen)
{
	CK_OBJECT_HANDLE hKey = CK_INVALID_HANDLE;
	generateAesKeyTest(IN_SESSION, IS_PUBLIC, 32, "testAesKey", hKey);
}

TEST_F(AesWrapUnwrapTest, aesWrapUnwrap)
{
	//hKey, Wrapping에 사용할 암호화 key준비
	CK_BBOOL bToken = CK_FALSE;		//IN_SESSION
	CK_BBOOL bPrivate = CK_FALSE;	//IS_PUBLIC

	CK_OBJECT_HANDLE hKey = CK_INVALID_HANDLE;
	generateAesKeyTest(bToken, bPrivate, 32, "testAesKey", hKey);

	//hSecret, Wrapping의 대상이 되는 객체 준비 (키는 C_GenerateRandom을 사용해서 생성한다.)
	CK_MECHANISM_TYPE mechanismType = CKM_AES_KEY_WRAP;
	CK_MECHANISM mechanism = { mechanismType, NULL_PTR, 0 };
	CK_BBOOL bFalse = CK_FALSE;
	CK_BBOOL bTrue = CK_TRUE;
	CK_OBJECT_CLASS secretClass = CKO_SECRET_KEY;
	CK_KEY_TYPE genKeyType = CKK_GENERIC_SECRET;
	CK_BYTE keyPtr[128];
	CK_ULONG keyLen = mechanismType == CKM_AES_KEY_WRAP_PAD ? 125UL : 128UL;
	CK_ATTRIBUTE attribs[] = {
		{ CKA_EXTRACTABLE, &bTrue, sizeof(bTrue) },
		{ CKA_CLASS, &secretClass, sizeof(secretClass) },
		{ CKA_KEY_TYPE, &genKeyType, sizeof(genKeyType) },
		{ CKA_TOKEN, &bToken, sizeof(bToken) },
		{ CKA_PRIVATE, &bPrivate, sizeof(bPrivate) },
		{ CKA_SENSITIVE, &bTrue, sizeof(bTrue) },	// Wrapping is allowed even on sensitive objects
		{ CKA_VALUE, keyPtr, keyLen }
	};

	EXPECT_EQ(C_GenerateRandom(_hSession, keyPtr, keyLen), CKR_OK);
	CK_OBJECT_HANDLE hSecret = CK_INVALID_HANDLE;
	EXPECT_EQ(C_CreateObject(_hSession, attribs, sizeof(attribs) / sizeof(CK_ATTRIBUTE), &hSecret), CKR_OK);
	EXPECT_NE(hSecret, CK_INVALID_HANDLE);

	//hSecret을 hKey를 사용하여 wrapping하여 암호화된 버퍼 wrappedPtr을 얻는다, (wrappedPtr은 keyPtr이 hKey의 키로 암호화된 버퍼다)
	CK_BYTE_PTR wrappedPtr = NULL_PTR;
	CK_ULONG wrappedLen = 0UL;
	CK_ULONG rndKeyLen = keyLen;
	EXPECT_EQ(C_WrapKey(_hSession, &mechanism, hKey, hSecret, NULL, &wrappedLen), CKR_OK);
	EXPECT_EQ(wrappedLen, rndKeyLen + 8);

	wrappedPtr = (CK_BYTE_PTR)malloc(wrappedLen);
	EXPECT_EQ(C_WrapKey(_hSession, &mechanism, hKey, hSecret, wrappedPtr, &wrappedLen), CKR_OK);
	EXPECT_EQ(wrappedLen, rndKeyLen + 8);

	//암호화된 wrappedPtr을 풀기 위하여 hKey를 사용하여 unwrap하고 그 결과를 담은 hNew객체를 리턴받는다
	CK_ATTRIBUTE nattribs[] = {
		{ CKA_CLASS, &secretClass, sizeof(secretClass) },
		{ CKA_KEY_TYPE, &genKeyType, sizeof(genKeyType) },
		{ CKA_TOKEN, &bFalse, sizeof(bFalse) },
		{ CKA_PRIVATE, &bFalse, sizeof(bTrue) },
		{ CKA_ENCRYPT, &bFalse, sizeof(bFalse) },
		{ CKA_DECRYPT, &bTrue, sizeof(bTrue) },
		{ CKA_SIGN, &bFalse, sizeof(bFalse) },
		{ CKA_VERIFY, &bTrue, sizeof(bTrue) },
		{ CKA_EXTRACTABLE, &bTrue, sizeof(bTrue) }
	};
	CK_OBJECT_HANDLE hNew = CK_INVALID_HANDLE;
	EXPECT_EQ(C_UnwrapKey(_hSession, &mechanism, hKey, wrappedPtr, wrappedLen, nattribs, sizeof(nattribs) / sizeof(CK_ATTRIBUTE), &hNew),CKR_OK);
	EXPECT_NE(hNew , CK_INVALID_HANDLE);

	CK_ATTRIBUTE keyAttribs[] = {
		{ CKA_VALUE, NULL_PTR, 0 }
	};

	//hNew객체의 속성, 키 버퍼값과 길이 값을 확인하여 원래의 keyPtr과 동일한 키가 hNew에 들어있음을 확인한다.
	EXPECT_EQ(C_GetAttributeValue(_hSession, hNew, keyAttribs, 1),CKR_OK);
	EXPECT_EQ(keyLen,keyAttribs[0].ulValueLen);

	keyAttribs[0].pValue = (CK_BYTE_PTR)malloc(keyAttribs[0].ulValueLen);
	EXPECT_EQ(C_GetAttributeValue(_hSession, hNew, keyAttribs, 1), CKR_OK);
	EXPECT_EQ(keyLen, keyAttribs[0].ulValueLen);
	EXPECT_EQ(memcmp(keyPtr,keyAttribs[0].pValue,keyLen),0);
	free(keyAttribs[0].pValue);

	free(wrappedPtr);
}

TEST_F(AesWrapUnwrapTest, aesKeyRetrieve)
{
	CK_ULONG keySize = 32;

	CK_OBJECT_HANDLE hKey = CK_INVALID_HANDLE;
	generateAesKeyTest(IN_SESSION, IS_PUBLIC, keySize, "testAesKey", hKey);

	CK_ATTRIBUTE keyAttribs[] = {
		{ CKA_VALUE, NULL_PTR, 0 }
	};

	EXPECT_EQ(C_GetAttributeValue(_hSession, hKey, keyAttribs, 1), CKR_OK);

	keyAttribs[0].pValue = (CK_BYTE_PTR)malloc(keyAttribs[0].ulValueLen);
	EXPECT_EQ(C_GetAttributeValue(_hSession, hKey, keyAttribs, 1), CKR_OK);
	EXPECT_EQ(keyAttribs[0].ulValueLen, keySize);
	free(keyAttribs[0].pValue);
}

//softhsm2 void ObjectTests::testCreateSecretKey() 참조
TEST_F(AesWrapUnwrapTest, aesKeyInjection)
{
	//1. 원하는 키를 만들어 그 값으로 객체를 생성한다.
	CK_ULONG keySize = 32;	//Max size
	CK_MECHANISM mechanism = { CKM_AES_KEY_GEN, NULL_PTR, 0 };
	CK_BBOOL bTrue = CK_TRUE;
	CK_BBOOL bFalse = CK_FALSE;
	CK_BYTE_PTR key = (CK_BYTE_PTR)malloc(keySize);
	for (CK_ULONG i = 0; i < keySize; i++) key[i] = (CK_BYTE)i;		//키값을 원하는 값으로 정의함

	CK_OBJECT_CLASS secretClass = CKO_SECRET_KEY;
	CK_KEY_TYPE keyType = CKK_GENERIC_SECRET;
	CK_ATTRIBUTE attribs[] = {
		{ CKA_VALUE, key, keySize },
		{ CKA_EXTRACTABLE, &bTrue, sizeof(bFalse) },
		{ CKA_CLASS, &secretClass, sizeof(secretClass) },
		{ CKA_KEY_TYPE, &keyType, sizeof(keyType) },
		{ CKA_TOKEN, &bFalse, sizeof(bFalse) },
		{ CKA_PRIVATE, &bTrue, sizeof(bTrue) },
		{ CKA_SENSITIVE, &bFalse, sizeof(bTrue) }
	};

	CK_OBJECT_HANDLE hKey = CK_INVALID_HANDLE;
	EXPECT_EQ(C_CreateObject(_hSession, attribs, sizeof(attribs) / sizeof(CK_ATTRIBUTE), &hKey), CKR_OK);

	//2. 객체에 키값을 읽어들여 그 값이 1에서 지정한 키 값인지 확인한다.
	CK_ATTRIBUTE keyAttribs[] = {
		{ CKA_VALUE, NULL_PTR, 0 }
	};

	EXPECT_EQ(C_GetAttributeValue(_hSession, hKey, keyAttribs, 1), CKR_OK);

	keyAttribs[0].pValue = (CK_BYTE_PTR)malloc(keyAttribs[0].ulValueLen);
	EXPECT_EQ(C_GetAttributeValue(_hSession, hKey, keyAttribs, 1), CKR_OK);
	EXPECT_EQ(keyAttribs[0].ulValueLen, keySize);
	EXPECT_EQ(memcmp(key, keyAttribs[0].pValue, keySize), 0);
	free(keyAttribs[0].pValue);

	//3. 위에서 생성된 객체(키)를 이용해서 암호화 및 복호화를 수행한다. (AES ECB encoding/decoding test)
	const int blockSize(0x10);
	const int NumBlock(10);
	CK_BYTE data[blockSize*NumBlock] = { "Billy Elliot the musical. What a fascinating experince! Love it" };

	//AES ECB encoding
	const CK_MECHANISM mechanismEnc = { CKM_AES_ECB, NULL_PTR, 0 };
	CK_MECHANISM_PTR pMechanism((CK_MECHANISM_PTR)&mechanismEnc);

	EXPECT_EQ(C_EncryptInit(_hSession, pMechanism, hKey),CKR_OK);

	CK_ULONG ulEncryptedDataLen;
	EXPECT_EQ(C_Encrypt(_hSession, data, sizeof(data), NULL_PTR, &ulEncryptedDataLen), CKR_OK);

	std::vector<CK_BYTE> vEncryptedData;
	vEncryptedData.resize(ulEncryptedDataLen);
	EXPECT_EQ(C_Encrypt(_hSession, data, sizeof(data), &vEncryptedData.front(), &ulEncryptedDataLen), CKR_OK);

	//AES ECB decoding
	EXPECT_EQ(C_DecryptInit(_hSession, pMechanism, hKey), CKR_OK);

	CK_ULONG ulDataLen;
	EXPECT_EQ(C_Decrypt(_hSession, &vEncryptedData.front(), (unsigned long)vEncryptedData.size(), NULL_PTR, &ulDataLen), CKR_OK);

	std::vector<CK_BYTE> vDecryptedData;
	vDecryptedData.resize(ulDataLen);
	EXPECT_EQ(C_Decrypt(_hSession, &vEncryptedData.front(), (unsigned long)vEncryptedData.size(), &vDecryptedData.front(), &ulDataLen), CKR_OK);

	//디코딩된 결과는 원래의 데이터와 같아야 한다.
	EXPECT_EQ(sizeof(data), ulDataLen);
	EXPECT_EQ(memcmp(data, &vDecryptedData.front(), sizeof(data)), 0);

	free(key);
}