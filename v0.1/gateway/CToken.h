#ifndef __CTOKEN_H__
#define __CTOKEN_H__

#include "cryptoki.h"

#define MAX_ERR_MSG 256

class CToken{
public:
	CToken();
	virtual ~CToken();

	int initialize();
	int slotCount(CK_ULONG &ulSlotCount);
	int initToken(CK_ULONG ulSlotCount, const char *soPin, const char *label);
	int openSession(CK_SLOT_ID slotID, CK_FLAGS flags = CKF_SERIAL_SESSION | CKF_RW_SESSION);

	char _message[MAX_ERR_MSG];

protected:
	void *_module;
	CK_FUNCTION_LIST_PTR _p11;
	CK_SESSION_HANDLE _hSession;
};

#endif