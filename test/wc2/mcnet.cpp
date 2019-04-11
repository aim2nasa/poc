#include <gtest/gtest.h>
#include "imcnet.h"

TEST(IMcNetTest, send) { 
    typedef unsigned char byte;

    class CTest : public IMcNet{
    public:
        ssize_t send(const void *buf, size_t len)
        {
            std::vector<byte> b;
            return b.size();
        }
    };

    CTest t;
	ASSERT_EQ(t.send("abc",sizeof("abc")), 3);
}
