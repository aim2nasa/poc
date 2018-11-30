#ifndef __FRAUDDETECT_H__
#define __FRAUDDETECT_H__

class FraudDetect{
public:
	FraudDetect();
	~FraudDetect();

    int init(int port,int backlog=1);
    int run();

    int sock_;
};

#endif
