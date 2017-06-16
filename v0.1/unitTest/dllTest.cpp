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
	//CKR_CRYPTOKI_ALREADY_INITIALIZED	(0x191) ���� �߻�
	//�ᱹ,dll�� softhsm2�� ������ �ν��Ͻ��� ��� �� ����.
	EXPECT_EQ(dll.init(USER_PIN),-1);
}