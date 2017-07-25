#ifndef __CTOKEN_H__
#define __CTOKEN_H__

#include "cryptoki.h"
#include <string>
#include <common.h>

#define MAX_ERR_MSG			256
#define INVALID_SLOT_ID		-1

class MODEXPORT CToken{
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
	int genRandom(char *randomData, unsigned long randomDataSize);
	int getKey(CK_OBJECT_HANDLE hKey, CK_BYTE_PTR key, CK_ULONG keySize);

	CK_SLOT_ID slotID();
	std::string label();
	CK_SESSION_HANDLE session();
	void slotID(CK_SLOT_ID slotID);
	int getSlotID();

	int createAesKey(CK_ATTRIBUTE *keyAttrib, CK_ULONG keyAttribNo, CK_ULONG keySize, CK_OBJECT_HANDLE &hKey);
	int deriveAesKey(CK_ATTRIBUTE *keyAttrib, CK_ULONG keyAttribNo, CK_OBJECT_HANDLE hKey, CK_OBJECT_HANDLE &hDerive, CK_MECHANISM_TYPE mechType, CK_BYTE *data, CK_LONG dataSize, CK_CHAR_PTR iv=NULL);

	char _message[MAX_ERR_MSG];

protected:
	void *_module;
	CK_FUNCTION_LIST_PTR _p11;
	CK_SESSION_HANDLE _hSession;
	CK_SLOT_ID _slotID;
	std::string _label;
};

#endif