#include "CHsmProxy.h"
#include "CToken.h"
#include "protocol.h"
#include "Configuration.h"
#include "OSPathSep.h"
#include "util.h"
#include "Directory.h"
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

int CHsmProxy::init(const char *userPin)
{
	int nRtn;
	if ( (nRtn=token_->initialize()) != 0) {
		sprintf_s(message_, MAX_ERR_MSG, "token initialize error(%d):%s",nRtn,token_->_message);
		return -1;
	}

	if ((nRtn = token_->getSlotID()) != 0) {
		sprintf_s(message_, MAX_ERR_MSG, "token getSlotID error(%d):%s", nRtn, token_->_message);
		return -2;
	}
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

int CHsmProxy::init(const char *label, const char *soPin, const char *userPin, bool emptySlot)
{
	int nRtn;
	if ((nRtn = token_->initialize()) != 0) {
		sprintf_s(message_, MAX_ERR_MSG, "token initialize error(%d):%s", nRtn, token_->_message);
		return -1;
	}

	CK_ULONG ulSlotCount;
	if ((nRtn=token_->slotCount(ulSlotCount)) != 0) {
		sprintf_s(message_, MAX_ERR_MSG, "token slotCount error(%d):%s", nRtn, token_->_message);
		return -2;
	}

	if (emptySlot && ulSlotCount != 1) {	//슬롯이 하나도 없어야 하는 조건일때 카운트는 1로 나와야 한다. 슬롯이 없을때 softhsm2는 1로 카운트해서 알려주기 때문
		sprintf_s(message_, MAX_ERR_MSG, "token emptySlot error:slotCount(%u)", ulSlotCount);
		return -3;
	}
	if (emptySlot) assert(ulSlotCount==1);

	if ((nRtn=token_->initToken(ulSlotCount - 1, soPin, (CK_ULONG)strlen(soPin), label, (CK_ULONG)strlen(label))) != 0) {
		sprintf_s(message_, MAX_ERR_MSG, "token initToken error(%d):%s", nRtn, token_->_message);
		return -4;
	}

	if ((nRtn=token_->openSession()) != 0) {
		sprintf_s(message_, MAX_ERR_MSG, "token openSession error(%d):%s", nRtn, token_->_message);
		return -5;
	}

	if ((nRtn=token_->login(CKU_SO, soPin, (CK_ULONG)strlen(soPin))) != 0) {
		sprintf_s(message_, MAX_ERR_MSG, "token SO login error(%d):%s", nRtn, token_->_message);
		return -6;
	}

	if ((nRtn=token_->initPin(userPin, (CK_ULONG)strlen(userPin))) != 0) {
		sprintf_s(message_, MAX_ERR_MSG, "token initPin error(%d):%s", nRtn, token_->_message);
		return -7;
	}

	if ((nRtn=token_->logout()) != 0) {
		sprintf_s(message_, MAX_ERR_MSG, "token logout error(%d):%s", nRtn, token_->_message);
		return -8;
	}

	if ((nRtn=token_->login(CKU_USER, userPin, (CK_ULONG)strlen(userPin))) != 0) {
		sprintf_s(message_, MAX_ERR_MSG, "token USER login error(%d):%s", nRtn, token_->_message);
		return -9;
	}
	return 0;
}

unsigned long CHsmProxy::slotID()
{
	return token_->slotID();
}

int CHsmProxy::findKey(const char *label, unsigned int labelSize, unsigned long &hKey)
{
	CK_ATTRIBUTE attribs[] = { { CKA_LABEL, (CK_UTF8CHAR_PTR)label, labelSize } };

	if(C_FindObjectsInit(token_->session(), &attribs[0], 1)!=CKR_OK) return -1;

	CK_SESSION_HANDLE hK = CK_INVALID_HANDLE;
	CK_ULONG ulObjectCount = 0;
	if (C_FindObjects(token_->session(), &hK, 1, &ulObjectCount) != CKR_OK) return -2;
	if (ulObjectCount > 1) return -3;	//하나의 키만을 찾게 된다는 전제를 하고 있음, 따라서 하나 이상의 경우에는 오류 처리
	if (ulObjectCount == 0) return -4;	//키를 발견하지 못한 경우임

	hKey = (unsigned long)hK;
	C_FindObjectsFinal(token_->session());
	return 0;
}

CToken& CHsmProxy::token()
{
	return *token_;
}

int CHsmProxy::deleteToken(char* slotSerialNo, char* tokenLabel)
{
	if (slotSerialNo == NULL && tokenLabel == NULL) return -1;

	if (!initSoftHSM()) {
		finalizeSoftHSM();
		return -2;
	}

	std::string basedir = Configuration::i()->getString("directories.tokendir", DEFAULT_TOKENDIR);
	std::string tokendir;

	if (findTokenDirectory(basedir, tokendir, slotSerialNo, tokenLabel))
	{
		std::string fulldir = basedir;
		if (fulldir.find_last_of(OS_PATHSEP) != (fulldir.size() - 1))
		{
			fulldir += OS_PATHSEP + tokendir;
		}
		else
		{
			fulldir += tokendir;
		}

		if (rmdir(fulldir))
		{
			return 0;
		}
		else{
			return -3;
		}
	}
	return -4;
}

int CHsmProxy::emptyToken()
{
	if (!initSoftHSM()) {
		finalizeSoftHSM();
		return -1;
	}

	std::string basedir = Configuration::i()->getString("directories.tokendir", DEFAULT_TOKENDIR);
	std::string tokendir;

	// Find all tokens in the specified path
	Directory storeDir(basedir);

	if (!storeDir.isValid())
	{
		fprintf(stderr, "Failed to enumerate object store in %s", basedir.c_str());
		return -2;
	}

	// Assume that all subdirectories are tokens
	std::vector<std::string> dirs = storeDir.getSubDirs();
	for (std::vector<std::string>::iterator i = dirs.begin(); i != dirs.end(); i++)
	{
		std::string fulldir = basedir;
		fulldir += *i;

		if (!rmdir(fulldir)){
			//삭제 실패 메세지
		}
	}
	return 0;
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

int CHsmProxy::setenv(const char *name, const char *value, int overwrite)
{
	std::string vv = name;
	vv += "=";
	vv += value;

	if (overwrite != 1)
		return false;

	return _putenv(vv.c_str()) == 0;
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