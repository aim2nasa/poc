#ifndef __FRAUDDETECT_H__
#define __FRAUDDETECT_H__

class FraudDetect{
public:
	FraudDetect();
	~FraudDetect();

    int init(int port,int backlog=0);
    int start(void *arg);

protected:
    static void* run(void *arg);
    int sock_;
};

#endif
