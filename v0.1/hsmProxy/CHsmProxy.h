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
	int findKey(const char *label, unsigned int labelSize, unsigned long &hKey);	//hKey�� Ÿ���� CK_SESSION_HANDLE������ pkcs11����� �������� �ʱ� ���ؼ� �⺻������ ǥ��
	CToken& token();
	static int deleteToken(char* slotSerialNo, char* tokenLabel);	//���Ͻý��ۿ� �����ϴ� ��ū�� ���̸����� ���� �Ǵ� �ø��� ��ȣ�� Ư�� ���Ը��� ����
	static int emptyToken();	//softhsm2.conf�� ������ ������ ���� �Լ�(HSM�� �����Ҷ� ��ū�� �����ϰ� ���� �Լ�)

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