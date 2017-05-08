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

	CK_SESSION_HANDLE _hSession;
};

CK_RV AesWrapUnwrapTest::generateAesKey(CK_BBOOL bToken, CK_BBOOL bPrivate, CK_ULONG bytes, const char *gw, CK_OBJECT_HANDLE &hKey)
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
		{ CKA_VALUE_LEN, &bytes, sizeof(bytes) }
	};

	hKey = CK_INVALID_HANDLE;
	return C_GenerateKey(_hSession, &mechanism, keyAttribs, sizeof(keyAttribs) / sizeof(CK_ATTRIBUTE), &hKey);
}

TEST_F(AesWrapUnwrapTest, sessionopen)
{
	EXPECT_NE(_hSession, CK_INVALID_HANDLE);
}

TEST_F(AesWrapUnwrapTest, aesKeyGen)
{
	EXPECT_NE(_hSession, CK_INVALID_HANDLE);
	CK_OBJECT_HANDLE hKey = CK_INVALID_HANDLE;
	EXPECT_EQ(generateAesKey(IN_SESSION, IS_PUBLIC, 32, "testAesKey", hKey), CKR_OK);
	EXPECT_NE(hKey, CK_INVALID_HANDLE);
}

TEST_F(AesWrapUnwrapTest, aesWrap)
{
	CK_BBOOL bToken = CK_FALSE;		//IN_SESSION
	CK_BBOOL bPrivate = CK_FALSE;	//IS_PUBLIC

	EXPECT_NE(_hSession, CK_INVALID_HANDLE);
	CK_OBJECT_HANDLE hKey = CK_INVALID_HANDLE;
	EXPECT_EQ(generateAesKey(bToken, bPrivate, 32, "testAesKey", hKey), CKR_OK);
	EXPECT_NE(hKey, CK_INVALID_HANDLE);

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

	CK_OBJECT_HANDLE hSecret = CK_INVALID_HANDLE;
	EXPECT_EQ(C_CreateObject(_hSession, attribs, sizeof(attribs) / sizeof(CK_ATTRIBUTE), &hSecret), CKR_OK);
	EXPECT_NE(hSecret, CK_INVALID_HANDLE);

	CK_BYTE_PTR wrappedPtr = NULL_PTR;
	CK_ULONG wrappedLen = 0UL;
	EXPECT_EQ(C_WrapKey(_hSession, &mechanism, hKey, hSecret, NULL, &wrappedLen), CKR_OK);
	EXPECT_NE(wrappedLen, 0);

	wrappedPtr = (CK_BYTE_PTR)malloc(wrappedLen);
	EXPECT_EQ(C_WrapKey(_hSession, &mechanism, hKey, hSecret, wrappedPtr, &wrappedLen), CKR_OK);
	EXPECT_NE(wrappedLen, 0);

	free(wrappedPtr);
}