#include "HsmTest.h"

class LibTest : public HsmTest{};

TEST_F(LibTest, loadLib)
{
	EXPECT_NE(this->_p11, reinterpret_cast<CK_FUNCTION_LIST_PTR>(NULL));
}