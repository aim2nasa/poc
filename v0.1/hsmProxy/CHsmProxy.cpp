#include "CHsmProxy.h"
#include "CToken.h"
#include "protocol.h"
#include <assert.h>

CHsmProxy::CHsmProxy()
:token_(NULL)
{
	token_ = new CToken();
}

CHsmProxy::~CHsmProxy()
{
	delete token_;
}

int CHsmProxy::init(const char *soPin, const char *userPin)
{
	if (token_->initialize() != 0) {
		sprintf_s(message_, MAX_ERR_MSG, "token initialize error");
		return -1;
	}

	if (token_->getSlotID() != 0) return -2;
	assert(token_->slotID() != INVALID_SLOT_ID);

	if (token_->openSession() != 0) {
		sprintf_s(message_, MAX_ERR_MSG, "token openSession error");
		return -3;
	}

	if (token_->login(CKU_USER, userPin, (CK_ULONG)strlen(userPin)) != 0) {
		sprintf_s(message_, MAX_ERR_MSG, "token user login error userPin:%s", userPin);
		return -4;
	}
	return 0;
}

int CHsmProxy::findKey(const char *label, unsigned int labelSize, unsigned long &hKey)
{
	CK_ATTRIBUTE attribs[] = { { CKA_LABEL, (CK_UTF8CHAR_PTR)label, labelSize } };

	if(C_FindObjectsInit(token_->session(), &attribs[0], 1)!=CKR_OK) return -1;

	CK_SESSION_HANDLE hK = CK_INVALID_HANDLE;
	CK_ULONG ulObjectCount = 0;
	if (C_FindObjects(token_->session(), &hK, 1, &ulObjectCount) != CKR_OK) return -2;
	if (ulObjectCount > 1) return -3;	//하나의 키만을 찾게 된다는 전제를 하고 있음, 따라서 하나 이상의 경우에는 오류 처리

	hKey = (unsigned long)hK;
	C_FindObjectsFinal(token_->session());
	return 0;
}

CToken& CHsmProxy::token()
{
	return *token_;
}