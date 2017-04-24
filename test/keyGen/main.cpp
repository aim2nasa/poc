#include <iostream>
#include "library.h"

using namespace std;

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

	//AES키를 생성
	CK_OBJECT_HANDLE hKey = CK_INVALID_HANDLE;
	{
		CK_MECHANISM mechanism = { CKM_AES_KEY_GEN, NULL_PTR, 0 };
		CK_ULONG bytes = 16;
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

		rv = C_GenerateKey(hSession, &mechanism, keyAttribs, sizeof(keyAttribs) / sizeof(CK_ATTRIBUTE), &hKey);
		if (rv != CKR_OK) {
			cout << "ERROR: C_GenerateKey: 0x" << hex << rv << endl;
			return -1;
		}
	}

	//Key Derivation
	{
		CK_MECHANISM mechanism = { CKM_AES_ECB_ENCRYPT_DATA, NULL_PTR, 0 };
		CK_KEY_DERIVATION_STRING_DATA param1;
		CK_BYTE data[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
			0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
			0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24,
			0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32
		};

		param1.pData = &data[0];
		param1.ulLen = sizeof(data);
		mechanism.pParameter = &param1;
		mechanism.ulParameterLen = sizeof(param1);

		CK_KEY_TYPE keyType = CKK_AES;			//ok
		//CK_KEY_TYPE keyType = CKK_GENERIC_SECRET;	//ok
		//CK_KEY_TYPE keyType = CKK_DES;			//0x10	CKR_ATTRIBUTE_READ_ONLY
		//CK_KEY_TYPE keyType = CKK_DES2;			//0x10	CKR_ATTRIBUTE_READ_ONLY
		//CK_KEY_TYPE keyType = CKK_DES3;			//0x10	CKR_ATTRIBUTE_READ_ONLY

		CK_MECHANISM mechEncrypt = { CKM_VENDOR_DEFINED, NULL_PTR, 0 };
		mechEncrypt.mechanism = keyType;
		CK_ULONG secLen = 32;

		CK_OBJECT_CLASS keyClass = CKO_SECRET_KEY;
		CK_BBOOL bTrue = CK_TRUE;
		CK_BBOOL bFalse = CK_FALSE;
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

		CK_OBJECT_HANDLE hDerivedKey = CK_INVALID_HANDLE;
		rv = C_DeriveKey(hSession, &mechanism, hKey, keyAttribs, sizeof(keyAttribs) / sizeof(CK_ATTRIBUTE), &hDerivedKey);
		if (rv != CKR_OK) {
			cout << "ERROR: C_DeriveKey: 0x" << hex << rv << endl;
			return -1;
		}
	}

	unloadLib(module);
	cout << "end" << endl;
	return 0;
}