#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <string.h>

struct message {
    message(): type(0),size(0){ memset(body,0,sizeof(body)); }
    long type;
    unsigned int size;
    char body[256];
};

#endif
