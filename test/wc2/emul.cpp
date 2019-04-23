#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "SafeQueue.h"
#include "ISession.h"

#define MULTICAST_GROUP "225.0.0.37"
#define MULTICAST_PORT  12345

typedef unsigned char byte;
typedef SafeQueue<std::vector<byte>> Que;

class VSender : public ISender{
public:
    VSender(){}
    virtual int init(const char *ip,ushort port)
    {
        group.clear();
        return 0;
    }
    virtual ssize_t send(const void *buf,size_t len)
    {
        std::vector<byte> b;
        for(size_t i=0;i<len;i++) b.push_back(reinterpret_cast<const char*>(buf)[i]);
        for(std::vector<Que*>::iterator it=group.begin();it!=group.end();it++) (*it)->enqueue(b);
        return b.size();
    }
    virtual int close() { return 0; }

    static std::vector<Que*> group;
};

std::vector<Que*> VSender::group;

class VReceiver : public IReceiver{
public:
    VReceiver(){}
    virtual int init(const char *ip,ushort port)
    {
        VSender::group.push_back(&q_);
        return 0;
    }
    virtual ssize_t recv(void *buf,size_t len)
    {
        if(q_.front().size()>len) return -1;

        std::vector<byte> b = q_.dequeue();
        memcpy(buf,b.data(),b.size());
        return b.size();
    }
    virtual int close() { return 0; }

    SafeQueue<std::vector<byte>> q_;
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

    ASSERT_EQ(si->close(),0);
    ASSERT_EQ(ri->close(),0);
}

TEST(EmulTest, realFakeTogether)
{
    Sender s;
    Receiver r;
    run(&s,&r);

    VSender vs;
    VReceiver vr;
    run(&vs,&vr);
}

void sendRecv(ISender *si,IReceiver *ri,const char* msg, size_t msgSize)
{
    ASSERT_EQ(si->send(msg,msgSize),msgSize);

    char buf[128];
    ASSERT_EQ(ri->recv(buf,sizeof(buf)),msgSize);
    ASSERT_EQ(memcmp(buf,msg,msgSize),0);
}

void runMulti(ISender *si,IReceiver *ri)
{
    ASSERT_EQ(si->init(MULTICAST_GROUP,MULTICAST_PORT),0);
    ASSERT_EQ(ri->init(MULTICAST_GROUP,MULTICAST_PORT),0);

    sendRecv(si,ri,"1",sizeof("1"));
    sendRecv(si,ri,"22",sizeof("22"));
    sendRecv(si,ri,"333",sizeof("333"));
    sendRecv(si,ri,"4444",sizeof("4444"));
    sendRecv(si,ri,"55555",sizeof("55555"));

    ASSERT_EQ(si->close(),0);
    ASSERT_EQ(ri->close(),0);
}

TEST(EmulTest, realMultiSendRecv)
{
    Sender s;
    Receiver r;
    runMulti(&s,&r);

    VSender vs;
    VReceiver vr;
    runMulti(&vs,&vr);
}

void helloMulticast(ISender *si,std::vector<IReceiver*> &ris)
{
    ASSERT_GT(ris.size(),0);

    ASSERT_EQ(si->init(MULTICAST_GROUP,MULTICAST_PORT),0);
    for(std::vector<IReceiver*>::iterator it=ris.begin();it!=ris.end();it++)
        ASSERT_EQ((*it)->init(MULTICAST_GROUP,MULTICAST_PORT),0);

    char msg[]={"HelloMulticast"};  //message to send
    size_t len=sizeof(msg);         //size of message to send
    ASSERT_EQ(len,strlen(msg)+1);   //sizeof accounts for NULL at the end

    ASSERT_EQ(si->send(msg,len),len);

    char buf[128];
    memset(buf,0,sizeof(buf));
    for(std::vector<IReceiver*>::iterator it=ris.begin();it!=ris.end();it++){
        ASSERT_EQ((*it)->recv(buf,sizeof(buf)),len);
        ASSERT_EQ(memcmp(buf,msg,len),0);
        memset(buf,0,sizeof(buf));
    }

    ASSERT_EQ(si->close(),0);
    for(std::vector<IReceiver*>::iterator it=ris.begin();it!=ris.end();it++)
        ASSERT_EQ((*it)->close(),0);
}

TEST(EmulTest, realMulticast)
{
    Sender s;
    Receiver r1,r2,r3;

    std::vector<IReceiver*> rs;
    rs.push_back(&r1);
    rs.push_back(&r2);
    rs.push_back(&r3);
    helloMulticast(&s,rs);
}

TEST(EmulTest, virtualMulticast)
{
    VSender s;
    VReceiver r1,r2,r3;

    std::vector<IReceiver*> rs;
    rs.push_back(&r1);
    rs.push_back(&r2);
    rs.push_back(&r3);
    helloMulticast(&s,rs);
}

void multicast(ISender *si,std::vector<IReceiver*> &ris,std::vector<std::string> &messages)
{
    ASSERT_GT(ris.size(),0);
    ASSERT_GT(messages.size(),0);

    ASSERT_EQ(si->init(MULTICAST_GROUP,MULTICAST_PORT),0);
    for(std::vector<IReceiver*>::iterator it=ris.begin();it!=ris.end();it++)
        ASSERT_EQ((*it)->init(MULTICAST_GROUP,MULTICAST_PORT),0);

    for(std::vector<std::string>::iterator it=messages.begin();it!=messages.end();it++){
        ASSERT_EQ(si->send((*it).c_str(),(*it).size()),(*it).size());

        char buf[128];
        ASSERT_GE(sizeof(buf),(*it).size());
        for(std::vector<IReceiver*>::iterator itr=ris.begin();itr!=ris.end();itr++){
            ASSERT_EQ((*itr)->recv(buf,sizeof(buf)),(*it).size());
            ASSERT_EQ(memcmp(buf,(*it).c_str(),(*it).size()),0);
#ifdef _DEBUG
            std::cout<<"Receiver="<<*itr<<",message="<<(*it).c_str()<<std::endl;
#endif
        }
    }

    ASSERT_EQ(si->close(),0);
    for(std::vector<IReceiver*>::iterator it=ris.begin();it!=ris.end();it++)
        ASSERT_EQ((*it)->close(),0);
}

TEST(EmulTest, realMulticastMessages)
{
    Sender s;
    Receiver r1,r2,r3;

    std::vector<IReceiver*> rs;
    rs.push_back(&r1);
    rs.push_back(&r2);
    rs.push_back(&r3);

    std::vector<std::string> msgs;
    msgs.push_back("message #1");
    msgs.push_back("message #2");
    msgs.push_back("message #3");

    multicast(&s,rs,msgs);
}

TEST(EmulTest, virtualMulticastMessages)
{
    VSender s;
    VReceiver r1,r2,r3;

    std::vector<IReceiver*> rs;
    rs.push_back(&r1);
    rs.push_back(&r2);
    rs.push_back(&r3);

    std::vector<std::string> msgs;
    msgs.push_back("message #1");
    msgs.push_back("message #2");
    msgs.push_back("message #3");

    multicast(&s,rs,msgs);
}
