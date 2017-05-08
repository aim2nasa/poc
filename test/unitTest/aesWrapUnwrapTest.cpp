#include "HsmTest.h"
#include "..\keyGen\helper.h"

class AesWrapUnwrapTest : public HsmTest{
protected:
	AesWrapUnwrapTest() :_hSession(CK_INVALID_HANDLE){}

	virtual void SetUp()
	{
		HsmTest::SetUp();
		EXPECT_NE(this->_p11, reinterpret_cast<CK_FUNCTION_LIST_PTR>(NULL));
		EXPECT_EQ(prepare(".\\softhsm2.conf", this->_p11, "1234", "MyToken", "1234", _hSession), 0);
	}

	virtual void TearDown()
	{
		HsmTest::TearDown();
	}

	CK_SESSION_HANDLE _hSession;
};

TEST_F(AesWrapUnwrapTest, sessionopen)
{
	EXPECT_NE(_hSession, CK_INVALID_HANDLE);
}