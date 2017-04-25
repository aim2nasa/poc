#include <iostream>
#include <assert.h>
#include "library.h"

using namespace std;

int aesDerive(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hKey, CK_OBJECT_HANDLE &hDerive, CK_MECHANISM_TYPE mechType, CK_BYTE *data, CK_LONG dataSize, CK_CHAR_PTR iv=NULL);

int main(int argc, const char* argv[])
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

	const char *soPin = argv[1];
	const char *label = argv[2];
	const char *userPin = argv[3];

	//토큰을 생성한다 (세션 오픈 포함)
	CK_SESSION_HANDLE hSession;
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

	//GwKey(AES키를 생성)
	CK_OBJECT_HANDLE hGw = CK_INVALID_HANDLE;
	{
		CK_MECHANISM mechanism = { CKM_AES_KEY_GEN, NULL_PTR, 0 };
		CK_ULONG bytes = 32;
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

		rv = C_GenerateKey(hSession, &mechanism, keyAttribs, sizeof(keyAttribs) / sizeof(CK_ATTRIBUTE), &hGw);
		if (rv != CKR_OK) {
			cout << "ERROR: C_GenerateKey: 0x" << hex << rv << endl;
			return -1;
		}
	}
	cout << "GW Key("<< hGw <<") ok" << endl;

	//Derive G1
	int nRtn;
	CK_OBJECT_HANDLE hG1 = CK_INVALID_HANDLE;
	CK_BYTE g1Data[32];
	memset(g1Data, 0, sizeof(g1Data));
	memcpy(g1Data, "Unique Name of G1", sizeof("Unique Name of G1"));
	nRtn = aesDerive(hSession, hGw, hG1, CKM_AES_ECB_ENCRYPT_DATA, g1Data, sizeof(g1Data));
	if (nRtn != 0) {
		cout << "ERROR: aesDerive: " << dec << ",rtn=" << nRtn << endl;
		return -1;
	}
	cout << "G1 Key(" << hG1 << ") ok" << endl;

	//Derive T1 from G1
	CK_OBJECT_HANDLE hT1 = CK_INVALID_HANDLE;

	CK_BYTE keyValue[64];
	CK_ATTRIBUTE valAttrib = { CKA_VALUE, &keyValue, sizeof(keyValue) };
	rv = C_GetAttributeValue(hSession, hG1, &valAttrib, 1);
	assert(rv == CKR_OK);

	nRtn = aesDerive(hSession, hG1, hT1, CKM_AES_ECB_ENCRYPT_DATA, (CK_BYTE*)valAttrib.pValue, valAttrib.ulValueLen);
	if (nRtn != 0) {
		cout << "ERROR: aesDerive: " << dec << ",rtn=" << nRtn << endl;
		return -1;
	}
	cout << "T1 Key(" << hT1 << ") ok" << endl;

	//Derive G11 from G1
	CK_OBJECT_HANDLE hG11 = CK_INVALID_HANDLE;

	CK_BYTE g11Data[32];
	memset(g11Data, 0, sizeof(g11Data));
	memcpy(g11Data, "Serial Number of G11", sizeof("Serial Number of G11"));
	nRtn = aesDerive(hSession, hG1, hG11, CKM_AES_ECB_ENCRYPT_DATA, g11Data, sizeof(g11Data));
	if (nRtn != 0) {
		cout << "ERROR: aesDerive: " << dec << ",rtn=" << nRtn << endl;
		return -1;
	}
	cout << "G11 Key(" << hG11 << ") ok" << endl;

	//Derive G12 from G1
	CK_OBJECT_HANDLE hG12 = CK_INVALID_HANDLE;

	CK_BYTE g12Data[32];
	memset(g12Data, 0, sizeof(g12Data));
	memcpy(g12Data, "Serial Number of G12", sizeof("Serial Number of G12"));
	nRtn = aesDerive(hSession, hG1, hG12, CKM_AES_ECB_ENCRYPT_DATA, g12Data, sizeof(g12Data));
	if (nRtn != 0) {
		cout << "ERROR: aesDerive: " << dec << ",rtn=" << nRtn << endl;
		return -1;
	}
	cout << "G12 Key(" << hG12 << ") ok" << endl;

	unloadLib(module);
	cout << "end" << endl;
	return 0;
}

int aesDerive(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hKey, CK_OBJECT_HANDLE &hDerive, CK_MECHANISM_TYPE mechType, CK_BYTE *data, CK_LONG dataSize, CK_CHAR_PTR iv)
{
	CK_RV rv;
	CK_MECHANISM mechanism = { mechType, NULL_PTR, 0 };
	CK_MECHANISM mechEncrypt = { CKM_VENDOR_DEFINED, NULL_PTR, 0 };
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
	mechEncrypt.mechanism = CKM_AES_ECB;
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