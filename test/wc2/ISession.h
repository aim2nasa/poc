#ifndef __ISESSION_H__
#define __ISESSION_H__

#include <linux/types.h>

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

#endif
