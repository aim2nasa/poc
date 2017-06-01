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

unsigned long CHsmProxy::mechanismType(MechanismType mType)
{
	//자체의 메카니즘타입을 softhsm에서 정의된 타입으로 매핑함 (HsmProxy에서 softhsm의 사용을 완전히 숨기기 위한 일환)
	CK_MECHANISM_TYPE mechanismType;
	switch (mType){
	case AES_CBC_PAD:
		mechanismType = CKM_AES_CBC_PAD;
		break;
	case AES_CBC:
		mechanismType = CKM_AES_CBC;
		break;
	case AES_ECB:
		mechanismType = CKM_AES_ECB;
		break;
	default:
		assert(false);	//정의되지 않은 타입이 들어옴
		break;
	}
	return mechanismType;
}

int CHsmProxy::encryptInit(MechanismType mType, unsigned long hKey)
{
	const CK_MECHANISM mechanismEnc = { mechanismType(mType), NULL_PTR, 0 };
	CK_MECHANISM_PTR pMechanism((CK_MECHANISM_PTR)&mechanismEnc);

	CK_RV rv;
	if ((rv = C_EncryptInit(token_->session(), pMechanism, hKey)) != CKR_OK) return rv;
	return 0;
}

int CHsmProxy::encrypt(unsigned char *data, unsigned long dataLen, unsigned char *encryptedData, unsigned long *encryptedDataLen)
{
	CK_RV rv;
	if ((rv = C_Encrypt(token_->session(), data, dataLen, encryptedData, encryptedDataLen)) != CKR_OK) return rv;
	return 0;
}

int CHsmProxy::decryptInit(MechanismType mType, unsigned long hKey)
{
	const CK_MECHANISM mechanismEnc = { mechanismType(mType), NULL_PTR, 0 };
	CK_MECHANISM_PTR pMechanism((CK_MECHANISM_PTR)&mechanismEnc);

	CK_RV rv;
	if ((rv = C_DecryptInit(token_->session(), pMechanism, hKey)) != CKR_OK) return rv;
	return 0;
}

int CHsmProxy::decrypt(unsigned char *encryptedData, unsigned long encryptedDataLen, unsigned char *data, unsigned long *dataLen)
{
	CK_RV rv;
	if ((rv = C_Decrypt(token_->session(), encryptedData, encryptedDataLen, data, dataLen)) != CKR_OK) return rv;
	return 0;
}