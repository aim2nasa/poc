#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "SafeQueue.h"

#define MULTICAST_GROUP "225.0.0.37"
#define MULTICAST_PORT  12345

typedef unsigned char byte;

struct ISession{
    virtual ~ISession() {}
    virtual int init(const char *ip,ushort port)=0;
    virtual int close()=0;
};

struct ISender : public ISession{
    virtual ~ISender() {}
    virtual ssize_t send(const void *buf,size_t len)=0;
};

struct IReceiver : public ISession{
    virtual ~IReceiver() {}
    virtual ssize_t recv(void *buf,size_t len)=0;
};

class VSender : public ISender{
public:
    VSender(){}
    virtual int init(const char *ip,ushort port) { return 0; }
    virtual ssize_t send(const void *buf,size_t len)
    {
        std::vector<byte> b;
        for(size_t i=0;i<len;i++) b.push_back(reinterpret_cast<const char*>(buf)[i]);
        q_.enqueue(b);
        return b.size();
    }
    virtual int close() { return 0; }

    SafeQueue<std::vector<byte>> q_;
};

class VReceiver : public IReceiver{
public:
    VReceiver(SafeQueue<std::vector<byte>> &q):q_(q){}
    virtual int init(const char *ip,ushort port) { return 0; }
    virtual ssize_t recv(void *buf,size_t len)
    {
        if(q_.front().size()>len) return -1;

        std::vector<byte> b = q_.dequeue();
        memcpy(buf,b.data(),b.size());
        return b.size();
    }
    virtual int close() { return 0; }

    SafeQueue<std::vector<byte>> &q_;
};

class Sender : public ISender{
public:
    Sender():fd_(-1){}
    virtual int init(const char *ip,ushort port) {
        if((fd_=socket(AF_INET,SOCK_DGRAM,0)) < 0) return -1;
   
        memset(&addr_,0,sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = inet_addr(ip);
        addr_.sin_port=port;
        return 0;
    }
    virtual ssize_t send(const void *buf,size_t len)
    {
        return sendto(fd_,buf,len,0,(struct sockaddr *) &addr_,sizeof(addr_)); 
    }
    virtual int close() { return ::close(fd_); }
private:
    int fd_;
    struct sockaddr_in addr_;
};

class Receiver : public IReceiver{
public:
    Receiver(int enable=1):fd_(-1),enable_(enable){}
    virtual int init(const char *ip,ushort port) {
        if((fd_=socket(AF_INET,SOCK_DGRAM,0)) < 0) return -1;
        if(setsockopt(fd_,SOL_SOCKET,SO_REUSEADDR,&enable_,sizeof(enable_)) < 0) return -2; 

        memset(&addr_,0,sizeof(addr_));
        addr_.sin_family=AF_INET;
        addr_.sin_addr.s_addr=htonl(INADDR_ANY);
        addr_.sin_port=port;

        if(bind(fd_,(struct sockaddr *)&addr_,sizeof(addr_)) < 0) return -3;

        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr=inet_addr(ip);
        mreq.imr_interface.s_addr=htonl(INADDR_ANY);
        if(setsockopt(fd_,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) return -4;

        return 0;
    }
    virtual ssize_t recv(void *buf,size_t len)
    {
        socklen_t addrlen = sizeof(addr_);
        return recvfrom(fd_,buf,len,0,(struct sockaddr *)&addr_,&addrlen);
    }
    virtual int close() { return ::close(fd_); }
private:
    int fd_,enable_;
    struct sockaddr_in addr_;
};

void run(ISender *si,IReceiver *ri)
{
    ASSERT_EQ(si->init(MULTICAST_GROUP,MULTICAST_PORT),0);
    ASSERT_EQ(ri->init(MULTICAST_GROUP,MULTICAST_PORT),0);

    char msg[]={"abcde"};   //message to send
    size_t len=sizeof(msg); //size of message to send
    ASSERT_EQ(len,6);

    ASSERT_EQ(si->send(msg,len),len);

    char buf[128];
    ASSERT_EQ(ri->recv(buf,sizeof(buf)),len);
    ASSERT_EQ(memcmp(buf,msg,len),0);
}

TEST(MockTest, realFakeTogether)
{
    Sender s;
    Receiver r;
    run(&s,&r);

    VSender vs;
    VReceiver vr(vs.q_);
    run(&vs,&vr);
}
