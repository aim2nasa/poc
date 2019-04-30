#include <gtest/gtest.h>
 
TEST(SimpleTest, comparison) { 
	ASSERT_EQ(0, 0);
}

TEST(StringTest, size) {
    std::string str("abc");
    ASSERT_EQ(str.size(),3);

    char tmp[]={0x41,0x42,0x43,0,0x45};
    ASSERT_EQ(sizeof(tmp),5);

    std::string tmpStr(tmp);
    ASSERT_EQ(tmpStr.size(),3); //because of 0 in the 4th
}
