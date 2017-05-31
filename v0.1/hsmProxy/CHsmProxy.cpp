#include "CHsmProxy.h"
#include "CToken.h"
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