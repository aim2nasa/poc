#include <gtest/gtest.h>
#include "imcnet.h"

TEST(IMcNetTest, send) { 
    typedef unsigned char byte;

    class CTest : public IMcNet{
    public:
        int join(IRecv *rcv) { return -1; }
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

TEST(IMcNetTest, sendRecv) { 
    typedef unsigned char byte;

    class CSender : public IMcNet{
    public:
        int join(IRecv *rcv)
        {
            rcv_ = rcv;
            return 0;
        }
        ssize_t send(const void *buf, size_t len)
        {
            std::vector<byte> b((byte*)buf,(byte*)buf+len);
            rcv_->recv(b.data(),b.size());
            return b.size();
        }
        IRecv *rcv_;
    };

    class CReceiver : public IRecv{
    public:
        ssize_t recv(void *buf,size_t len)
        {
            std::cout<<"CReceiver::recv="<<len<<std::endl;
            return len;
        }
    }; 

    CReceiver r;
    CSender s; 
    ASSERT_EQ(s.join(&r),0);
    ASSERT_EQ(s.send("abc",3),3);
}
