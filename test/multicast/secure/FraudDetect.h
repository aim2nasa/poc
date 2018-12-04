#ifndef __FRAUDDETECT_H__
#define __FRAUDDETECT_H__

#include <vector>
#include "message.h"

#define MAX_QUEUE 5

class FraudDetect{
public:
	FraudDetect();
	~FraudDetect();

    int init(int port,int backlog=0);
    int start(void *arg);

    int msqid_;
protected:
    static void* run(void *arg);
    static bool exist(std::vector<message>& q,const char *buff,unsigned int buffSize);
    int sock_;
};

#endif
