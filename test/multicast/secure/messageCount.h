#ifndef __MESSAGECOUNT_H__
#define __MESSAGECOUNT_H__

#include "message.h"

struct messageCount : public message {
    messageCount():visitCount(0){ }
    int visitCount;
};

#endif
