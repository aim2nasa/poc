#ifndef __CTESTDLL_H__
#define __CTESTDLL_H__

class CHsmProxy;

class __declspec(dllexport) CTestDll{
public:
	CTestDll();
	virtual ~CTestDll();

	int init(const char *soPin, const char *userPin);

	CHsmProxy	*hsm_;
};

#endif