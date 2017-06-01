#ifndef __CHSMPROXY_H__
#define __CHSMPROXY_H__

#define MAX_ERR_MSG 256

class CToken;

class __declspec(dllexport) CHsmProxy{
public:
	CHsmProxy();
	virtual ~CHsmProxy();

	int init(const char *soPin, const char *userPin);
	int findKey(const char *label, unsigned int labelSize, unsigned long &hKey);	//hKey의 타입은 CK_SESSION_HANDLE이지만 pkcs11헤더를 포함하지 않기 위해서 기본형으로 표시
	CToken& token();

	enum MechanismType {
		AES_CBC_PAD,
		AES_CBC,
		AES_ECB
	};
	static unsigned long mechanismType(MechanismType mType);
	int encryptInit(MechanismType mType, unsigned long hKey);
	int encrypt(unsigned char *data, unsigned long dataLen, unsigned char *encryptedData, unsigned long *encryptedDataLen);
	int decryptInit(MechanismType mType, unsigned long hKey);
	int decrypt(unsigned char *encryptedData, unsigned long encryptedDataLen, unsigned char *data, unsigned long *dataLen);

	char message_[MAX_ERR_MSG];
private:
	CToken *token_;
};

#endif