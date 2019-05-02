#ifndef __FRAUDDETECT_H__
#define __FRAUDDETECT_H__

#include <vector>
#include "messageCount.h"
#include <entity/node.h>

#define MAX_QUEUE 5

class FraudDetect{
public:
	FraudDetect();
	~FraudDetect();

    int init(int port,int backlog=0);
    int start(void *arg);
    static void* run(void *arg);
    void setKeys(int size,int key,int iv);

    int msqid_;
protected:
    static bool exist(std::vector<messageCount>& q,const char *buff,unsigned int buffSize);
    static int existOrder(std::vector<messageCount>& q,const char *buff,unsigned int buffSize);
    int sock_;
    Node Bob_;
};

#endif
