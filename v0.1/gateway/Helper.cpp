#include "Helper.h"
#include "CToken.h"
#include <ace/Log_Msg.h>

int prepareSession(CToken &token, const char *label, const char *soPin, const char *userPin)
{
	if (token.initialize() != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}
	ACE_DEBUG((LM_INFO, "(%t) HSM library initialized\n"));

	CK_ULONG ulSlotCount;
	if (token.slotCount(ulSlotCount) != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}
	ACE_DEBUG((LM_INFO, "(%t) number of slots:%d\n", ulSlotCount));

	if (token.initToken(ulSlotCount - 1, soPin, (CK_ULONG)strlen(soPin), label, (CK_ULONG)strlen(label)) != 0) { //slotID: 디폴트로 들어가는 한개의 카운트를 제외한다. 슬롯이 하나도 없을때도 카운트는 1로 나오므로
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}

	if (token.openSession() != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}

	if (token.login(CKU_SO, soPin, (CK_ULONG)strlen(soPin)) != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}

	if (token.initPin(userPin, (CK_ULONG)strlen(userPin)) != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}

	if (token.logout() != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}

	if (token.login(CKU_USER, userPin, (CK_ULONG)strlen(userPin)) != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}

	ACE_RETURN(0);
}

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