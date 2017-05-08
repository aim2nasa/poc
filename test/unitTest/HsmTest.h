#ifndef __HSM_TEST_H__
#define __HSM_TEST_H__

#include <gtest/gtest.h>
#include "library.h"

class HsmTest : public ::testing::Test{
protected:
	HsmTest() :_module(NULL),_p11(NULL){}
	virtual ~HsmTest(){}

	virtual void SetUp()
	{
		EXPECT_EQ(loadLibOnly(&_module, &_p11),0);
		EXPECT_EQ(_p11->C_Initialize(NULL_PTR), CKR_OK);
	}

	virtual void TearDown()
	{
		unloadLib(_module);
	}

	void* _module;
public:
	CK_FUNCTION_LIST_PTR _p11;
};

#endif