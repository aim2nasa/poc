#include <gtest/gtest.h>
#include "imcnet.h"

typedef unsigned char byte;

TEST(IMcNetTest, send) { 
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
            for(size_t i=0;i<len;i++) std::cout<<*(reinterpret_cast<char*>(buf)+i);
            std::cout<<std::endl;
            return len;
        }
    }; 

    CReceiver r;
    CSender s; 
    ASSERT_EQ(s.join(&r),0);
    ASSERT_EQ(s.send("abc",3),3);
}

TEST(IMcNetTest, set) { 
    std::set<IRecv*> tb;

    class CReceiver : public IRecv{
        ssize_t recv(void *buf,size_t len) {return 0;}
    };
    
    CReceiver c1;

    std::pair<std::set<IRecv*>::iterator,bool> ret;

    ret=tb.insert(&c1);
    ASSERT_EQ(ret.second,true);
    ret=tb.insert(&c1);
    ASSERT_EQ(ret.second,false); //already exist

    CReceiver c2,c3;
    ret=tb.insert(&c2);
    ASSERT_EQ(ret.second,true);
    ret=tb.insert(&c3);
    ASSERT_EQ(ret.second,true);

    ASSERT_EQ(tb.size(),3);
}

TEST(IMcNetTest, multiReceivers)
{
    class CSender : public IMcNet{
    public:
        int join(IRecv *rcv)
        {
            std::pair<std::set<IRecv*>::iterator,bool> ret = tb_.insert(rcv);
            if(ret.second==false) return -1;
            return 0;
        }
        ssize_t send(const void *buf, size_t len)
        {
            std::vector<byte> b((byte*)buf,(byte*)buf+len);
            for(std::set<IRecv*>::iterator it=tb_.begin();it!=tb_.end();++it)
                (*it)->recv(b.data(),b.size());
                
            return b.size();
        }
        std::set<IRecv*> tb_;
    };

    class CReceiver : public IRecv{
    public:
        CReceiver(std::string name):name_(name){}
        ssize_t recv(void *buf,size_t len)
        {
            std::cout<<"CReceiver("<<name_<<")::recv="<<len<<std::endl;
            for(size_t i=0;i<len;i++) std::cout<<*(reinterpret_cast<char*>(buf)+i);
            std::cout<<std::endl;
            return len;
        }
        std::string name_;
    }; 

    CReceiver r1("R1"),r2("R2"),r3("R3");
    CSender s;
    ASSERT_EQ(s.join(&r1),0);
    ASSERT_EQ(s.join(&r1),-1);  //already exist
    ASSERT_EQ(s.join(&r2),0);
    ASSERT_EQ(s.join(&r3),0);
    ASSERT_EQ(s.tb_.size(),3);

    ASSERT_EQ(s.send("abc",3),3);
}
