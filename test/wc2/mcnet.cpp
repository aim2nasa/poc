#include <gtest/gtest.h>
#include "imcnet.h"

TEST(IMcNetTest, send) { 
    typedef unsigned char byte;

    class CTest : public IMcNet{
    public:
        ssize_t send(const void *buf, size_t len)
        {
            std::vector<byte> b((byte*)buf,(byte*)buf+len);
            return b.size();
        }
    };

	ASSERT_EQ(sizeof("abc"), 4);
    CTest t;
	ASSERT_EQ(t.send("abc",3), 3);
}
