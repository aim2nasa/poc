#include <gtest/gtest.h>
#include "CTestDll.h"
#include "CHsmProxy.h"
#include "testConf.h"

TEST(DllTest, simple)
{
	CHsmProxy p;
	p.setenv("SOFTHSM2_CONF", ".\\softhsm2.conf", 1);
	ASSERT_EQ(p.init(USER_PIN), 0);

	CTestDll dll;
	//CKR_CRYPTOKI_ALREADY_INITIALIZED	(0x191) 오류 발생
	//결국,dll로 softhsm2를 별도의 인스턴스로 띄울 수 없다.
	EXPECT_EQ(dll.init(USER_PIN),-1);
}