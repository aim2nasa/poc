#include "Helper.h"
#include "CToken.h"
#include "common.h"
#include <ace/Log_Msg.h>

//GwKey(AES키를 생성)
int gatewayKey(CToken &token, CK_ULONG keySize, CK_OBJECT_HANDLE &hGw)
{
	hGw = CK_INVALID_HANDLE;
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
		{ CKA_VALUE_LEN, &keySize, sizeof(keySize) }
	};
	if (token.createAesKey(keyAttribs, sizeof(keyAttribs) / sizeof(CK_ATTRIBUTE), keySize, hGw) != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("createAesKey failed\n")));
		ACE_RETURN(-1);
	}
	ACE_RETURN(0);
}

int aesDerive(CToken &token, CK_OBJECT_HANDLE hKey, CK_OBJECT_HANDLE &hDerive, CK_MECHANISM_TYPE mechType, CK_BYTE *data, CK_LONG dataSize, CK_CHAR_PTR iv)
{
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
	if (token.deriveAesKey(keyAttribs, sizeof(keyAttribs) / sizeof(CK_ATTRIBUTE), hKey, hDerive, mechType, data, dataSize, iv) != 0) {
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("deriveAesKey failed\n")), -1);
	}
	ACE_RETURN(0);
}

//Derive Group
int deriveGroup(CToken &token, CK_OBJECT_HANDLE &hGroup, CK_OBJECT_HANDLE hParent, CK_BYTE_PTR data, CK_ULONG dataSize)
{
	hGroup = CK_INVALID_HANDLE;
	int nRtn;
	if ((nRtn = aesDerive(token, hParent, hGroup, CKM_AES_ECB_ENCRYPT_DATA, data, dataSize)) != 0)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("aesDerive failed:0x%x\n"), nRtn), -1);

	ACE_RETURN(0);
}

//Derive Tag from Group
int deriveTagFromGroup(CToken &token, CK_OBJECT_HANDLE &hTag, CK_OBJECT_HANDLE hGroup)
{
	hTag = CK_INVALID_HANDLE;

	CK_BYTE keyValue[64];
	CK_ATTRIBUTE valAttrib = { CKA_VALUE, &keyValue, sizeof(keyValue) };
	CK_RV rv = C_GetAttributeValue(token.session(), hGroup, &valAttrib, 1);
	if (rv != CKR_OK)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: C_GetAttributeValue:0x%x\n"), rv), -1);

	int nRtn;
	if ((nRtn = aesDerive(token, hGroup, hTag, CKM_AES_ECB_ENCRYPT_DATA, (CK_BYTE*)valAttrib.pValue, valAttrib.ulValueLen)) != 0)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: aesDerive:0x%x\n"), nRtn), -2);
	
	ACE_RETURN(0);
}

int showKey(CToken &token, CK_OBJECT_HANDLE hKey, const char *name)
{
	CK_BYTE key[AES_KEY_SIZE];
	if (token.getKey(hKey, key, sizeof(key)) != 0) ACE_RETURN(-1);

	displayKey(key, sizeof(key), name);
	ACE_RETURN(0);
}

void displayKey(CK_BYTE_PTR key, CK_ULONG keySize, const char *name)
{
	ACE_DEBUG((LM_INFO, ACE_TEXT("(%t) %s key:"), name));
	for (unsigned long i = 0; i < keySize; i++) ACE_DEBUG((LM_INFO, ACE_TEXT("%0x "), key[i]));
	ACE_DEBUG((LM_INFO, ACE_TEXT("\n")));
}