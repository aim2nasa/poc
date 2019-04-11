#ifndef __IMCNET_H__
#define __IMCNET_H__

#include <sys/types.h>

class IMcNet{
public:
    virtual ~IMcNet(){}

    virtual ssize_t send(const void *buf, size_t len)=0;
};

#endif
