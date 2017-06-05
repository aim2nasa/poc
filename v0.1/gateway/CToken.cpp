#include "CToken.h"
#include "library.h"
#include <stdio.h>
#include <memory.h>
#include <vector>

CToken::CToken()
:_module(NULL), _p11(NULL), _hSession(CK_INVALID_HANDLE), _slotID(INVALID_SLOT_ID)
{

}

CToken::~CToken()
{
	if (_p11) _p11->C_Finalize(NULL_PTR);
	if (!_module) unloadLib(_module);
}

int CToken::initialize()
{
	if (loadLibOnly(&_module, &_p11) == -1) {
		sprintf_s(_message, MAX_ERR_MSG, "%s", "ERROR: loadLib");
		return -1;
	}

	CK_RV rv;
	if ( (rv=_p11->C_Initialize(NULL_PTR)) != CKR_OK) {
		sprintf_s(_message, MAX_ERR_MSG, "%s:0x%x", "ERROR: C_Initialize",rv);
		return -2;
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

int CToken::initToken(CK_SLOT_ID slotID, const char *soPin, CK_ULONG soPinize, const char *label, CK_ULONG labelSize)
{
	if (slotID == INVALID_SLOT_ID || soPin == NULL || label == NULL) {
		sprintf_s(_message, MAX_ERR_MSG, "%s", "ERROR: wrong argument");
		return -1;
	}

	_slotID = slotID;
	_label = label;

	CK_UTF8CHAR paddedLabel[32];
	memset(paddedLabel, ' ', sizeof(paddedLabel));
	memcpy(paddedLabel, label, labelSize);

	CK_RV rv;
	if ((rv = _p11->C_InitToken(_slotID, (CK_UTF8CHAR_PTR)soPin, soPinize, paddedLabel) != CKR_OK)) {
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

int CToken::logout()
{
	CK_RV rv;
	if ((rv = _p11->C_Logout(_hSession)) != CKR_OK) {
		sprintf_s(_message, MAX_ERR_MSG, "%s %x", "ERROR: C_Logout: 0x", rv);
		return -1;
	}
	return 0;
}

CK_SLOT_ID CToken::slotID()
{
	return _slotID;
}

std::string CToken::label()
{
	return _label;
}

CK_SESSION_HANDLE CToken::session()
{
	return _hSession;
}

void CToken::slotID(CK_SLOT_ID slotID)
{
	_slotID = slotID;
}

int CToken::getSlotID()
{
	bool hasFoundInitialized(false);

	CK_ULONG nrOfSlots;
	if (C_GetSlotList(CK_TRUE, NULL_PTR, &nrOfSlots) != CKR_OK) return -1;

	std::vector<CK_SLOT_ID> slotIDs(nrOfSlots);
	if (C_GetSlotList(CK_TRUE, &slotIDs.front(), &nrOfSlots) != CKR_OK) return -2;

	for (std::vector<CK_SLOT_ID>::iterator i = slotIDs.begin(); i != slotIDs.end(); i++) {
		CK_TOKEN_INFO tokenInfo;

		if (C_GetTokenInfo(*i, &tokenInfo) != CKR_OK) return -3;
		if (tokenInfo.flags & CKF_TOKEN_INITIALIZED) {
			if (!hasFoundInitialized) {
				hasFoundInitialized = true;
				_slotID = *i;
				return 0;
			}
		}
	}
	return -1;
}

int CToken::createAesKey(CK_ATTRIBUTE *keyAttrib, CK_ULONG keyAttribNo, CK_ULONG keySize, CK_OBJECT_HANDLE &hKey)
{
	hKey = CK_INVALID_HANDLE;
	CK_MECHANISM mechanism = { CKM_AES_KEY_GEN, NULL_PTR, 0 };
	CK_RV rv = C_GenerateKey(_hSession, &mechanism, keyAttrib, keyAttribNo, &hKey);
	if (rv != CKR_OK) {
		sprintf_s(_message, MAX_ERR_MSG, "%s %x", "ERROR: C_Logout: 0x", rv);
		return -1;
	}
	return 0;
}

int CToken::genRandom(char *randomData, unsigned long randomDataSize)
{
	CK_RV rv;
	if ((rv = _p11->C_GenerateRandom(_hSession, reinterpret_cast<unsigned char*>(randomData), randomDataSize)) != CKR_OK) {
		sprintf_s(_message, MAX_ERR_MSG, "%s %x", "ERROR: C_GenerateRandom: 0x", rv);
		return -1;
	}
	return 0;
}