#include "CTestDll.h"
#include "CHsmProxy.h"

CTestDll::CTestDll()
:hsm_(0)
{
	hsm_ = new CHsmProxy();
}

CTestDll::~CTestDll()
{
	delete hsm_;
}

int CTestDll::init(const char *userPin)
{
	return hsm_->init(userPin);
}