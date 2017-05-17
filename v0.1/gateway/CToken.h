#ifndef __CTOKEN_H__
#define __CTOKEN_H__

#include "cryptoki.h"
#include <string>

#define MAX_ERR_MSG 256

class CToken{
public:
	CToken();
	virtual ~CToken();

	int initialize();
	int slotCount(CK_ULONG &ulSlotCount);
	int initToken(CK_SLOT_ID slotID, const char *soPin, CK_ULONG soPinize, const char *label, CK_ULONG labelSize);
	int openSession(CK_FLAGS flags = CKF_SERIAL_SESSION | CKF_RW_SESSION);
	int login(CK_USER_TYPE userType, const char *pin, CK_ULONG pinSize);
	int initPin(const char *userPin, CK_ULONG userPinSize);
	int logout();

	CK_SLOT_ID slotID();
	std::string label();

	int createAesKey(CK_ATTRIBUTE *keyAttrib, CK_ULONG keyAttribNo, CK_ULONG keySize, CK_OBJECT_HANDLE &hKey);

	char _message[MAX_ERR_MSG];

protected:
	void *_module;
	CK_FUNCTION_LIST_PTR _p11;
	CK_SESSION_HANDLE _hSession;
	CK_SLOT_ID _slotID;
	std::string _label;
};

#endif