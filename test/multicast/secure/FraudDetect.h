#ifndef __FRAUDDETECT_H__
#define __FRAUDDETECT_H__

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
    int sock_;
};

#endif
