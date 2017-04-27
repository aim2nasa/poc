#include "helper.h"
#include <iostream>
#include <assert.h>
#include <iomanip>

using namespace std;

void memInit(CK_BYTE_PTR data, CK_ULONG dataSize, const char *name)
{
	assert(strlen(name) <= dataSize);
	memset(data, 0, dataSize);
	memcpy(data, name, strlen(name));
}

//토큰을 생성한다 (세션 오픈 포함)
int prepare(CK_FUNCTION_LIST_PTR p11, const char *soPin, const char *label, const char *userPin, CK_SESSION_HANDLE &hSession)
{
	CK_ULONG ulSlotCount;
	CK_RV rv = p11->C_GetSlotList(CK_FALSE, NULL_PTR, &ulSlotCount);
	if (rv != CKR_OK) {
		cout << "ERROR: Couldn't get the number of slots: 0x" << hex << rv << endl;
		return -1;
	}
	cout << "number of slots:" << ulSlotCount << endl;

	CK_SLOT_ID slotID = ulSlotCount - 1; //디폴트로 들어가는 한개의 카운트를 제외한다. 슬롯이 하나도 없을때도 카운트는 1로 나오므로
	cout << "SlotID : 0x" << hex << slotID << dec << " (" << slotID << ")" << endl;

	CK_UTF8CHAR paddedLabel[32];
	memset(paddedLabel, ' ', sizeof(paddedLabel));
	memcpy(paddedLabel, label, strlen(label));

	rv = p11->C_InitToken(slotID, (CK_UTF8CHAR_PTR)soPin, (CK_ULONG)strlen(soPin), paddedLabel);
	if (rv != CKR_OK) {
		cout << "ERROR: C_InitToken: 0x" << hex << rv << endl;
		return -1;
	}

	rv = p11->C_OpenSession(slotID, CKF_SERIAL_SESSION | CKF_RW_SESSION, NULL_PTR, NULL_PTR, &hSession);
	if (rv != CKR_OK) {
		cout << "ERROR: C_OpenSession: 0x" << hex << rv << endl;
		return -1;
	}

	rv = p11->C_Login(hSession, CKU_SO, (CK_UTF8CHAR_PTR)soPin, (CK_ULONG)strlen(soPin));
	if (rv != CKR_OK) {
		cout << "ERROR: C_Login: 0x" << hex << rv << endl;
		return -1;
	}

	rv = p11->C_InitPIN(hSession, (CK_UTF8CHAR_PTR)userPin, (CK_ULONG)strlen(userPin));
	if (rv != CKR_OK) {
		cout << "ERROR: C_InitPIN: 0x" << hex << rv << endl;
		return -1;
	}

	rv = p11->C_Logout(hSession);
	if (rv != CKR_OK) {
		cout << "ERROR: C_Logout: 0x" << hex << rv << endl;
		return -1;
	}

	rv = p11->C_Login(hSession, CKU_USER, (CK_UTF8CHAR_PTR)userPin, (CK_ULONG)strlen(userPin));
	if (rv != CKR_OK) {
		cout << "ERROR: C_Login(USER): 0x" << hex << rv << endl;
		return -1;
	}
	return 0;
}

//GwKey(AES키를 생성)
int createAesKey(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE &hKey, CK_ULONG bytes, const char *gw)
{
	hKey = CK_INVALID_HANDLE;
	CK_MECHANISM mechanism = { CKM_AES_KEY_GEN, NULL_PTR, 0 };
	CK_BBOOL bTrue = CK_TRUE;
	CK_BBOOL bFalse = CK_FALSE;
	CK_ATTRIBUTE keyAttribs[] = {
		{ CKA_TOKEN, &bTrue, sizeof(bTrue) },
		{ CKA_PRIVATE, &bTrue, sizeof(bTrue) },
		{ CKA_EXTRACTABLE, &bTrue, sizeof(bTrue) },
		{ CKA_SENSITIVE, &bFalse, sizeof(bTrue) },
		{ CKA_DERIVE, &bTrue, sizeof(bTrue) },
		{ CKA_ENCRYPT, &bTrue, sizeof(bTrue) },
		{ CKA_DECRYPT, &bTrue, sizeof(bTrue) },
		{ CKA_WRAP, &bTrue, sizeof(bTrue) },
		{ CKA_UNWRAP, &bTrue, sizeof(bTrue) },
		{ CKA_VALUE_LEN, &bytes, sizeof(bytes) }
	};

	CK_RV rv = C_GenerateKey(hSession, &mechanism, keyAttribs, sizeof(keyAttribs) / sizeof(CK_ATTRIBUTE), &hKey);
	if (rv != CKR_OK) {
		cout << "ERROR: C_GenerateKey: 0x" << hex << rv << endl;
		return -1;
	}

	printKey(hSession, hKey);
	cout << gw << " Key(" << hKey << ") ok" << endl;
	return 0;
}

//Derive Group
int deriveGroup(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE &hGroup, CK_OBJECT_HANDLE hParent, CK_BYTE_PTR data, CK_ULONG dataSize, const char *group)
{
	hGroup = CK_INVALID_HANDLE;
	int nRtn;
	if ((nRtn = aesDerive(hSession, hParent, hGroup, CKM_AES_ECB_ENCRYPT_DATA, data, dataSize)) != 0) {
		cout << "ERROR: aesDerive: " << dec << ",rtn=" << nRtn << endl;
		return -1;
	}
	printKey(hSession, hGroup);
	cout << group << " Key(" << hGroup << ") ok" << endl;
	return 0;
}

//Derive Tag from Group
int deriveTagFromGroup(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE &hTag, CK_OBJECT_HANDLE hGroup, const char *tag)
{
	hTag = CK_INVALID_HANDLE;

	CK_BYTE keyValue[64];
	CK_ATTRIBUTE valAttrib = { CKA_VALUE, &keyValue, sizeof(keyValue) };
	CK_RV rv = C_GetAttributeValue(hSession, hGroup, &valAttrib, 1);
	if (rv != CKR_OK)
		return -1;

	int nRtn;
	if ((nRtn = aesDerive(hSession, hGroup, hTag, CKM_AES_ECB_ENCRYPT_DATA, (CK_BYTE*)valAttrib.pValue, valAttrib.ulValueLen)) != 0) {
		cout << "ERROR: aesDerive: " << dec << ",rtn=" << nRtn << endl;
		return -2;
	}
	printKey(hSession, hTag);

	assert(tag);
	cout << tag << " Key(" << hTag << ") ok" << endl;
	return 0;
}

int aesDerive(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hKey, CK_OBJECT_HANDLE &hDerive, CK_MECHANISM_TYPE mechType, CK_BYTE *data, CK_LONG dataSize, CK_CHAR_PTR iv)
{
	CK_RV rv;
	CK_MECHANISM mechanism = { mechType, NULL_PTR, 0 };
	CK_KEY_DERIVATION_STRING_DATA param1;
	CK_AES_CBC_ENCRYPT_DATA_PARAMS param3;

	switch (mechType)
	{
	case CKM_AES_ECB_ENCRYPT_DATA:
		param1.pData = data;
		param1.ulLen = dataSize;
		mechanism.pParameter = &param1;
		mechanism.ulParameterLen = sizeof(param1);
		break;
	case CKM_AES_CBC_ENCRYPT_DATA:
		assert(iv);
		memcpy(param3.iv, iv, 16);
		param3.pData = data;
		param3.length = dataSize;
		mechanism.pParameter = &param3;
		mechanism.ulParameterLen = sizeof(param3);
		break;
	default:
		return -1; //Invalid mechanism
	}

	CK_KEY_TYPE keyType = CKK_AES;
	CK_ULONG secLen = dataSize;	//8*32 = 256bit

	CK_OBJECT_CLASS keyClass = CKO_SECRET_KEY;
	CK_BBOOL bFalse = CK_FALSE;
	CK_BBOOL bTrue = CK_TRUE;
	CK_ATTRIBUTE keyAttribs[] = {
		{ CKA_CLASS, &keyClass, sizeof(keyClass) },
		{ CKA_KEY_TYPE, &keyType, sizeof(keyType) },
		{ CKA_PRIVATE, &bFalse, sizeof(bFalse) },
		{ CKA_ENCRYPT, &bTrue, sizeof(bTrue) },
		{ CKA_DECRYPT, &bTrue, sizeof(bTrue) },
		{ CKA_DERIVE, &bTrue, sizeof(bTrue) },
		{ CKA_SENSITIVE, &bFalse, sizeof(bFalse) },
		{ CKA_EXTRACTABLE, &bTrue, sizeof(bTrue) },
		{ CKA_VALUE_LEN, &secLen, sizeof(secLen) }
	};

	hDerive = CK_INVALID_HANDLE;
	rv = C_DeriveKey(hSession, &mechanism, hKey, keyAttribs, sizeof(keyAttribs) / sizeof(CK_ATTRIBUTE), &hDerive);
	if (rv != CKR_OK) return -3;

	return 0;
}

CK_RV printKey(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hKey)
{
	CK_BYTE keyValue[64];
	CK_ATTRIBUTE valAttrib = { CKA_VALUE, &keyValue, sizeof(keyValue) };
	CK_RV rv = C_GetAttributeValue(hSession, hKey, &valAttrib, 1);
	if (rv == CKR_OK) {
		cout << "\tKey(" << hKey << "):";
		for (unsigned long i = 0; i < valAttrib.ulValueLen; i++) cout << hex << setw(2) << (int)keyValue[i] << " ";
		cout << endl;
	}
	return rv;
}