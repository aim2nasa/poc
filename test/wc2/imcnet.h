#ifndef __IMCNET_H__
#define __IMCNET_H__

#include <sys/types.h>

class IRecv{
public:
    virtual ~IRecv(){};

    virtual ssize_t recv(void *buf,size_t len)=0;
};

class IMcNet{
public:
    virtual ~IMcNet(){}

    virtual int join(IRecv *rcv)=0;
    virtual ssize_t send(const void *buf, size_t len)=0;
};

#endif
