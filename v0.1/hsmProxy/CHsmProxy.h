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

	char message_[MAX_ERR_MSG];
private:
	CToken *token_;
};

#endif