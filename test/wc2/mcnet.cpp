#include <gtest/gtest.h>
#include "imcnet.h"

typedef unsigned char byte;

TEST(IMcNetTest, send) { 
    class CMcNet : public IMcNet{
    public:
        int join(IRecv *rcv) { return -1; }
        ssize_t send(const void *buf, size_t len)
        {
            std::vector<byte> b((byte*)buf,(byte*)buf+len);
            return b.size();
        }
    };

	ASSERT_EQ(sizeof("abc"), 4);
    CMcNet t;
	ASSERT_EQ(t.send("abc",3), 3);
}

TEST(IMcNetTest, sendRecv) { 
    class CMcNet : public IMcNet{
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
            reinterpret_cast<char*>(buf)[len]=0;
            str_ = reinterpret_cast<char*>(buf);
            len_ = len;
            return len;
        }
        std::string str_;
        size_t len_;
    }; 

    CReceiver r;
    CMcNet s; 
    ASSERT_EQ(s.join(&r),0);
    ASSERT_EQ(s.send("abc",3),3);
    ASSERT_EQ(r.str_,"abc");
    ASSERT_EQ(r.len_,3);
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
    class CMcNet : public IMcNet{
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
            reinterpret_cast<char*>(buf)[len]=0;
            str_ = reinterpret_cast<char*>(buf);
            len_ = len;
            return len;
        }
        std::string name_;
        std::string str_;
        size_t len_;
    }; 

    CReceiver r1("R1"),r2("R2"),r3("R3");
    CMcNet s;
    ASSERT_EQ(s.join(&r1),0);
    ASSERT_EQ(s.join(&r1),-1);  //already exist
    ASSERT_EQ(s.join(&r2),0);
    ASSERT_EQ(s.join(&r3),0);
    ASSERT_EQ(s.tb_.size(),3);

    ASSERT_EQ(s.send("abc",3),3);

    ASSERT_EQ(r1.name_,"R1");
    ASSERT_EQ(r1.str_,"abc");
    ASSERT_EQ(r1.len_,3);
    ASSERT_EQ(r2.name_,"R2");
    ASSERT_EQ(r2.str_,"abc");
    ASSERT_EQ(r2.len_,3);
    ASSERT_EQ(r3.name_,"R3");
    ASSERT_EQ(r3.str_,"abc");
    ASSERT_EQ(r3.len_,3);
}

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MULTICAST_GROUP "225.0.0.37"
#define MULTICAST_PORT  12345

class ICallback{
public:
    virtual ~ICallback(){}
    virtual ssize_t data(void *buf,size_t len)=0;
};

class CRcv{
public:
    CRcv():fd_(-1),enable_(1),buf_(0),bufSize_(0),thread_(0),cb_(0),status_(0){}
    ~CRcv(){ delete [] buf_; close(fd_); }

    int init(const char *ip,ushort port,ICallback *cb,size_t bufSize=256)
    {
        if((fd_=socket(AF_INET,SOCK_DGRAM,0))<0) return -1;
        if(setsockopt(fd_,SOL_SOCKET,SO_REUSEADDR,&enable_,sizeof(enable_)) < 0) return -1;

        memset(&addr_,0,sizeof(addr_));
        addr_.sin_family=AF_INET;
        addr_.sin_addr.s_addr=htonl(INADDR_ANY);
        addr_.sin_port=port;
        if(bind(fd_,(struct sockaddr *) &addr_,sizeof(addr_))<0) return -1;

        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr=inet_addr(ip);
        mreq.imr_interface.s_addr=htonl(INADDR_ANY);
        if(setsockopt(fd_,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) return -1;

        bufSize_ = bufSize;
        buf_ = new char[bufSize_];
        cb_ = cb;
        return 0;
    }

    ssize_t recv(void *buf,size_t len)
    {
        socklen_t addrlen = sizeof(addr_);
        return recvfrom(fd_,buf,len,0,(struct sockaddr *)&addr_,&addrlen);
    }

    int start()
    {
        return pthread_create(&thread_,NULL,run,this);
    }

    static void* run(void *arg)
    {
        CRcv* p = static_cast<CRcv*>(arg);

        ssize_t rcv;
        if((rcv=p->recv(p->buf_,p->bufSize_))<0) {
        }else{
            if(p->cb_) p->cb_->data(p->buf_,rcv);
        }
        return 0;
    }

    int wait()
    {
        return pthread_join(thread_,(void**)&status_);
    }

    int fd_;
    int enable_;
    struct sockaddr_in addr_;
    char *buf_;
    size_t bufSize_;
    pthread_t thread_;
    ICallback *cb_;
    int status_;
};

class CSend{
public:
    CSend():fd_(-1){}
    ~CSend(){ close(fd_); }

    int init(const char *ip,ushort port)
    {
        if((fd_=socket(AF_INET,SOCK_DGRAM,0))<0) return -1;

        memset(&addr_,0,sizeof(addr_));
        addr_.sin_family=AF_INET;
        addr_.sin_addr.s_addr=inet_addr(MULTICAST_GROUP);
        addr_.sin_port=port;

        return 0;
    }

    ssize_t send(const void *buf,size_t len)
    {
        return sendto(fd_,buf,len,0,(struct sockaddr *) &addr_,sizeof(addr_));
    }

    int fd_;
    struct sockaddr_in addr_;
};

class CCb : public ICallback{
public:
    ssize_t data(void *buf,size_t len)
    {
        reinterpret_cast<char*>(buf)[len]=0;
        str_=reinterpret_cast<char*>(buf);
        len_ = len;
    }

    std::string str_;
    size_t len_;
};

TEST(IMcNetTest, multicast)
{
    CCb cb;

    CRcv r;
    ASSERT_EQ(r.init(MULTICAST_GROUP,MULTICAST_PORT,&cb),0);
    ASSERT_EQ(r.start(),0);

    CSend s;
    ASSERT_EQ(s.init(MULTICAST_GROUP,MULTICAST_PORT),0);
    ASSERT_EQ(s.send("abcde",5),5);

    ASSERT_EQ(r.wait(),0);

    ASSERT_EQ(cb.str_,"abcde");
    ASSERT_EQ(cb.len_,5);
}

#include "gmock/gmock.h"

class MockCb : public ICallback{
public:
    MOCK_METHOD2(data,ssize_t(void *buf,size_t len));
};

TEST(MockTest, basic)
{
    MockCb m;
    EXPECT_CALL(m,data(0,0)).Times(1);
}
