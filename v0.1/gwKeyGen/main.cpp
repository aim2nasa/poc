#include <iostream>
#include <assert.h>
#include "library.h"
#include "createToken.h"
#include "generateRSA.h"

using namespace std;

CK_RV generateAesKey(CK_SESSION_HANDLE hSession, CK_BBOOL bToken, CK_BBOOL bPrivate, CK_OBJECT_HANDLE &hKey);
void symDerive(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hKey, CK_OBJECT_HANDLE &hDerive, CK_MECHANISM_TYPE mechType, CK_KEY_TYPE keyType);

int main(int argc, char* argv[])
{
	if (argc < 4) {
		cout << "usage: testpkcs11 <soPin> <label> <userPin>" << endl;
		return -1;
	}

	void* module;
	CK_FUNCTION_LIST_PTR p11 = NULL;
	if (loadLib(&module, &p11) == -1) {
		cout << "ERROR: loadLib" << endl;
		return -1;
	}
	cout << "loadLib ok" << endl;

	char *soPin = argv[1];
	char *label = argv[2];
	char *userPin = argv[3];

	//토큰을 생성한다 (세션 오픈 포함)
	CK_SESSION_HANDLE hSession;
	int nRtn = 0;;
	if ((nRtn = createToken(p11, &hSession, soPin, label, userPin)) != 0) {
		cout << "ERROR: createToken(" << soPin << "," << label << "," << userPin << ")=" << nRtn << endl;
		return -1;
	}
	cout << "token(" << label << ") created" << endl;

	//RSA Key pair를 생성한다
#if 0
	cout << "generating RSA key pair..." << endl;
	CK_OBJECT_HANDLE hPuk, hPrk;
	if ((nRtn = generateRsaKeyPair(hSession, ON_TOKEN, IS_PUBLIC, ON_TOKEN, IS_PUBLIC, &hPuk, &hPrk)) != 0) {
		cout << "ERROR: generateRSA=" << nRtn << endl;
		return -1;
	}
	cout << "RSA key pair generated" << endl;
#endif // 0


	//AES키를 생성
	CK_OBJECT_HANDLE hKeyAes = CK_INVALID_HANDLE;
	CK_RV rv = generateAesKey(hSession, ON_TOKEN, IS_PUBLIC, hKeyAes);
	assert(rv == CKR_OK);

	// Derive keys
	CK_OBJECT_HANDLE hDerive = CK_INVALID_HANDLE;
	symDerive(hSession, hKeyAes, hDerive, CKM_AES_ECB_ENCRYPT_DATA, CKK_AES);

	unloadLib(module);
	cout << "end" << endl;
	return 0;
}

CK_RV generateAesKey(CK_SESSION_HANDLE hSession, CK_BBOOL bToken, CK_BBOOL bPrivate, CK_OBJECT_HANDLE &hKey)
{
	CK_MECHANISM mechanism = { CKM_AES_KEY_GEN, NULL_PTR, 0 };
	CK_ULONG bytes = 16;
	// CK_BBOOL bFalse = CK_FALSE;
	CK_BBOOL bTrue = CK_TRUE;
	CK_ATTRIBUTE keyAttribs[] = {
		{ CKA_TOKEN, &bToken, sizeof(bToken) },
		{ CKA_PRIVATE, &bPrivate, sizeof(bPrivate) },
		{ CKA_SENSITIVE, &bTrue, sizeof(bTrue) },
		{ CKA_DERIVE, &bTrue, sizeof(bTrue) },
		{ CKA_VALUE_LEN, &bytes, sizeof(bytes) }
	};

	hKey = CK_INVALID_HANDLE;
	return C_GenerateKey(hSession, &mechanism,
		keyAttribs, sizeof(keyAttribs) / sizeof(CK_ATTRIBUTE),
		&hKey);
}

void symDerive(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hKey, CK_OBJECT_HANDLE &hDerive, CK_MECHANISM_TYPE mechType, CK_KEY_TYPE keyType)
{
	CK_RV rv;
	CK_MECHANISM mechanism = { mechType, NULL_PTR, 0 };
	CK_MECHANISM mechEncrypt = { CKM_VENDOR_DEFINED, NULL_PTR, 0 };
	CK_KEY_DERIVATION_STRING_DATA param1;
	CK_DES_CBC_ENCRYPT_DATA_PARAMS param2;
	CK_AES_CBC_ENCRYPT_DATA_PARAMS param3;

	CK_BYTE data[] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
		0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
		0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24,
		0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32
	};
	CK_ULONG secLen = 0;

	switch (mechType)
	{
	case CKM_DES_ECB_ENCRYPT_DATA:
	case CKM_DES3_ECB_ENCRYPT_DATA:
	case CKM_AES_ECB_ENCRYPT_DATA:
		param1.pData = &data[0];
		param1.ulLen = sizeof(data);
		mechanism.pParameter = &param1;
		mechanism.ulParameterLen = sizeof(param1);
		break;
	case CKM_DES_CBC_ENCRYPT_DATA:
	case CKM_DES3_CBC_ENCRYPT_DATA:
		memcpy(param2.iv, "12345678", 8);
		param2.pData = &data[0];
		param2.length = sizeof(data);
		mechanism.pParameter = &param2;
		mechanism.ulParameterLen = sizeof(param2);
		break;
	case CKM_AES_CBC_ENCRYPT_DATA:
		memcpy(param3.iv, "1234567890ABCDEF", 16);
		param3.pData = &data[0];
		param3.length = sizeof(data);
		mechanism.pParameter = &param3;
		mechanism.ulParameterLen = sizeof(param3);
		break;
	default:
		assert(0); //Invalid mechanism
	}

	switch (keyType)
	{
	case CKK_GENERIC_SECRET:
		secLen = 32;
		break;
	case CKK_DES:
		mechEncrypt.mechanism = CKM_DES_ECB;
		break;
	case CKK_DES2:
	case CKK_DES3:
		mechEncrypt.mechanism = CKM_DES3_ECB;
		break;
	case CKK_AES:
		mechEncrypt.mechanism = CKM_AES_ECB;
		secLen = 32;
		break;
	default:
		assert(0);	//Invalid key type
	}

	CK_OBJECT_CLASS keyClass = CKO_SECRET_KEY;
	CK_BBOOL bFalse = CK_FALSE;
	CK_BBOOL bTrue = CK_TRUE;
	CK_ATTRIBUTE keyAttribs[] = {
		{ CKA_CLASS, &keyClass, sizeof(keyClass) },
		{ CKA_KEY_TYPE, &keyType, sizeof(keyType) },
		{ CKA_PRIVATE, &bFalse, sizeof(bFalse) },
		{ CKA_ENCRYPT, &bTrue, sizeof(bTrue) },
		{ CKA_DECRYPT, &bTrue, sizeof(bTrue) },
		{ CKA_SENSITIVE, &bFalse, sizeof(bFalse) },
		{ CKA_EXTRACTABLE, &bTrue, sizeof(bTrue) },
		{ CKA_VALUE_LEN, &secLen, sizeof(secLen) }
	};

	hDerive = CK_INVALID_HANDLE;
	if (secLen > 0)
	{
		rv = C_DeriveKey(hSession, &mechanism, hKey,
			keyAttribs, sizeof(keyAttribs) / sizeof(CK_ATTRIBUTE),
			&hDerive);
	}
	else
	{
		rv = C_DeriveKey(hSession, &mechanism, hKey,
			keyAttribs, sizeof(keyAttribs) / sizeof(CK_ATTRIBUTE)-1,
			&hDerive);
	}
	assert(rv == CKR_OK);

	// Check that KCV has been set
	CK_ATTRIBUTE checkAttribs[] = {
		{ CKA_CHECK_VALUE, NULL_PTR, 0 }
	};
	CK_BYTE check[3];
	checkAttribs[0].pValue = check;
	checkAttribs[0].ulValueLen = sizeof(check);
	rv = C_GetAttributeValue(hSession, hDerive, checkAttribs, 1);
	assert(rv == CKR_OK);
	assert(checkAttribs[0].ulValueLen == 3);

	if (keyType == CKK_GENERIC_SECRET) return;

	CK_BYTE cipherText[300];
	CK_ULONG ulCipherTextLen;
	CK_BYTE recoveredText[300];
	CK_ULONG ulRecoveredTextLen;

	rv = C_EncryptInit(hSession, &mechEncrypt, hDerive);
	assert(rv == CKR_OK);

	ulCipherTextLen = sizeof(cipherText);
	rv = C_Encrypt(hSession, data, sizeof(data), cipherText, &ulCipherTextLen);
	assert(rv == CKR_OK);

	rv = C_DecryptInit(hSession, &mechEncrypt, hDerive);
	assert(rv == CKR_OK);

	ulRecoveredTextLen = sizeof(recoveredText);
	rv = C_Decrypt(hSession, cipherText, ulCipherTextLen, recoveredText, &ulRecoveredTextLen);
	assert(rv == CKR_OK);
	assert(ulRecoveredTextLen == sizeof(data));

	assert(memcmp(data, recoveredText, sizeof(data)) == 0);
}