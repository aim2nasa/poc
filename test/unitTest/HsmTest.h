#ifndef __HSM_TEST_H__
#define __HSM_TEST_H__

#include <gtest/gtest.h>
#include "library.h"

class HsmTest : public ::testing::Test{
protected:
	HsmTest() :_moduleHandle(NULL), _p11(NULL){}
	virtual ~HsmTest(){}

	virtual void SetUp()
	{
		char *module = NULL;
		char message[256];
		char *msg = message;
		CK_C_GetFunctionList pGetFunctionList = loadLibrary(module, &_moduleHandle, &msg);
		ASSERT_NE(pGetFunctionList, reinterpret_cast<CK_C_GetFunctionList>(NULL));

		(*pGetFunctionList)(&_p11);
		ASSERT_NE(_p11, reinterpret_cast<CK_FUNCTION_LIST_PTR>(NULL));

		_p11->C_Finalize(NULL_PTR);
		ASSERT_EQ(_p11->C_Initialize(NULL_PTR), CKR_OK);
	}

	virtual void TearDown()
	{
		ASSERT_NE(_moduleHandle, reinterpret_cast<void*>(NULL));
		unloadLibrary(_moduleHandle);
	}

	void *_moduleHandle;
public:
	CK_FUNCTION_LIST_PTR _p11;
};

#endif