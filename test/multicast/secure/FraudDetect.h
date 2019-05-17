#ifndef __FRAUDDETECT_H__
#define __FRAUDDETECT_H__

#include <vector>
#include "messageCount.h"
#include <entity/node.h>

#define MAX_QUEUE 5

struct vcRtn{
    vcRtn():visitCount(-1),order(-1){}
    int visitCount;
    int order;
};

class IDetect;

class FraudDetect{
public:
	FraudDetect();
	~FraudDetect();

    int init(int port,int backlog=0);
    int start(void *arg);
    static void* run(void *arg);
    void setKeys(int size,int key,int iv);
    int getFrameNumDiff(unsigned int frameNumber);
    static void verbosity(bool b);
    void setDetect(IDetect *p);

    int msqid_;
protected:
    static vcRtn getVisitCount(std::vector<messageCount>& q,const char *buff,unsigned int buffSize);
    int sock_;
    Node Bob_;
    bool prevFrameDefined_;
    unsigned int prevFrameNumber_;
    static bool verbosity_;
    IDetect *detect_;
};

#endif
