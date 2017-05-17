#include "CToken.h"
#include "library.h"
#include <stdio.h>
#include <memory.h>
#include <string.h>

#define INVALID_SLOT_ID		-1

CToken::CToken()
:_module(NULL), _p11(NULL), _hSession(CK_INVALID_HANDLE), _slotID(INVALID_SLOT_ID)
{

}

CToken::~CToken()
{
	if (!_module) unloadLib(_module);
}

int CToken::initialize()
{
	if (loadLibOnly(&_module, &_p11) == -1) {
		sprintf_s(_message, MAX_ERR_MSG, "%s", "ERROR: loadLib");
		return -1;
	}

	if (_p11->C_Initialize(NULL_PTR) != CKR_OK) {
		sprintf_s(_message, MAX_ERR_MSG, "%s", "ERROR: C_Initialize");
		return -1;
	}
	return 0;
}

int CToken::slotCount(CK_ULONG &ulSlotCount)
{
	CK_RV rv;
	if ((rv=_p11->C_GetSlotList(CK_FALSE, NULL_PTR, &ulSlotCount))!= CKR_OK) {
		sprintf_s(_message, MAX_ERR_MSG, "%s %x", "ERROR: Couldn't get the number of slots: 0x",rv);
		return -1;
	}
	return 0;
}

int CToken::initToken(CK_SLOT_ID slotID, const char *soPin, const char *label)
{
	if (slotID == INVALID_SLOT_ID || soPin == NULL || label == NULL) {
		sprintf_s(_message, MAX_ERR_MSG, "%s", "ERROR: wrong argument");
		return -1;
	}

	_slotID = slotID;

	CK_UTF8CHAR paddedLabel[32];
	memset(paddedLabel, ' ', sizeof(paddedLabel));
	memcpy(paddedLabel, label, strlen(label));

	CK_RV rv;
	if ((rv = _p11->C_InitToken(_slotID, (CK_UTF8CHAR_PTR)soPin, (CK_ULONG)strlen(soPin), paddedLabel) != CKR_OK)) {
		sprintf_s(_message, MAX_ERR_MSG, "%s %x", "ERROR: C_InitToken: 0x",rv);
		return -1;
	}
	return 0;
}

int CToken::openSession(CK_FLAGS flags)
{
	if (_slotID == INVALID_SLOT_ID) {
		sprintf_s(_message, MAX_ERR_MSG, "%s", "ERROR: slot id must be set prior through initToken");
		return -1;
	}

	CK_RV rv;
	if ((rv = _p11->C_OpenSession(_slotID, flags, NULL_PTR, NULL_PTR, &_hSession) != CKR_OK)) {
		sprintf_s(_message, MAX_ERR_MSG, "%s %x", "ERROR: C_OpenSession: 0x", rv);
		return -1;
	}
	return 0;
}

int CToken::login(CK_USER_TYPE userType, const char *pin, CK_ULONG pinSize)
{
	CK_RV rv;
	if ((rv = _p11->C_Login(_hSession, userType, (CK_UTF8CHAR_PTR)pin, pinSize)) != CKR_OK) {
		sprintf_s(_message, MAX_ERR_MSG, "%s %x", "ERROR: C_Login: 0x", rv);
		return -1;
	}
	return 0;
}

int CToken::initPin(const char *userPin, CK_ULONG userPinSize)
{
	CK_RV rv;
	if ((rv = _p11->C_InitPIN(_hSession, (CK_UTF8CHAR_PTR)userPin, userPinSize)) != CKR_OK) {
		sprintf_s(_message, MAX_ERR_MSG, "%s %x", "ERROR: C_InitPIN: 0x", rv);
		return -1;
	}
	return 0;
}