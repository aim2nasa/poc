#include <gtest/gtest.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
 
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

using namespace CryptoPP;

TEST(StringTest, stringSourceSink)
{
    char abc[]={0x41,0x42,0x43,0x44,0x45};
    std::string str(abc);
    std::string encoded;

    StringSource(str,true,
        new HexEncoder(
            new StringSink(encoded)
        )
    );
    ASSERT_EQ("4142434445",encoded);
}
