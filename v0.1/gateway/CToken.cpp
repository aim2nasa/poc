#include "CToken.h"
#include "library.h"
#include <stdio.h>
#include <memory.h>
#include <string.h>

CToken::CToken()
:_module(NULL), _p11(NULL)
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

int CToken::initToken(CK_ULONG ulSlotCount, const char *soPin, const char *label)
{
	if (ulSlotCount == 0 || soPin == NULL || label == NULL) {
		sprintf_s(_message, MAX_ERR_MSG, "%s", "ERROR: wrong argument");
		return -1;
	}

	CK_SLOT_ID slotID = ulSlotCount - 1;

	CK_UTF8CHAR paddedLabel[32];
	memset(paddedLabel, ' ', sizeof(paddedLabel));
	memcpy(paddedLabel, label, strlen(label));

	CK_RV rv;
	if ((rv = _p11->C_InitToken(slotID, (CK_UTF8CHAR_PTR)soPin, (CK_ULONG)strlen(soPin), paddedLabel) != CKR_OK)) {
		sprintf_s(_message, MAX_ERR_MSG, "%s %x", "ERROR: C_InitToken: 0x",rv);
		return -1;
	}
	return 0;
}