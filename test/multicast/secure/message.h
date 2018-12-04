#ifndef __MESSAGE_H__
#define __MESSAGE_H__

struct message {
    long type;
    unsigned int size;
    char body[256];
};

#endif
