#ifndef __CHSMPROXY_H__
#define __CHSMPROXY_H__

#define MAX_ERR_MSG 256

class CToken;

class __declspec(dllexport) CHsmProxy{
public:
	CHsmProxy();
	virtual ~CHsmProxy();

	int init(const char *soPin, const char *userPin);

	char message_[MAX_ERR_MSG];
private:
	CToken *token_;
};

#endif