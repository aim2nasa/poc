#ifndef __CHSMPROXY_H__
#define __CHSMPROXY_H__

#include <common.h>

#define MAX_ERR_MSG 256

class CToken;

class EXPORT CHsmProxy{
public:
	CHsmProxy();
	virtual ~CHsmProxy();

	int init(const char *userPin);
	int init(const char *label, const char *soPin, const char *userPin, bool emptySlot=true);
	unsigned long slotID();
	int findKey(const char *label, unsigned int labelSize, unsigned long &hKey);	//hKey의 타입은 CK_SESSION_HANDLE이지만 pkcs11헤더를 포함하지 않기 위해서 기본형으로 표시
	CToken& token();
	static int deleteToken(char* slotSerialNo, char* tokenLabel);	//파일시스템에 존재하는 토큰을 라벨이름으로 삭제 또는 시리얼 번호로 특정 슬롯만을 삭제
	static int emptyToken();	//softhsm2.conf에 지정된 폴더를 비우는 함수(HSM을 시작할때 토큰을 깨끗하게 비우는 함수)

	enum MechanismType {
		AES_CBC_PAD,
		AES_CBC,
		AES_ECB
	};
	static unsigned long mechanismType(MechanismType mType);
	int setenv(const char *name, const char *value, int overwrite);
	int encryptInit(MechanismType mType, unsigned long hKey);
	int encrypt(unsigned char *data, unsigned long dataLen, unsigned char *encryptedData, unsigned long *encryptedDataLen);
	int decryptInit(MechanismType mType, unsigned long hKey);
	int decrypt(unsigned char *encryptedData, unsigned long encryptedDataLen, unsigned char *data, unsigned long *dataLen);

	char message_[MAX_ERR_MSG];
private:
	CToken *token_;
};

#endif