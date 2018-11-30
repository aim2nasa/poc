#ifndef __FRAUDDETECT_H__
#define __FRAUDDETECT_H__

class FraudDetect{
public:
	FraudDetect();
	~FraudDetect();

    int init(int port,int backlog=0);
    int run();

    int sock_;
};

#endif
