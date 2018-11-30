#ifndef __FRAUDDETECT_H__
#define __FRAUDDETECT_H__

#define OK              0
#define ERROR_SOCKET    -10
#define ERROR_BIND      -11
#define ERROR_LISTEN    -12
#define ERROR_RECEIVE   -13

class FraudDetect{
public:
	FraudDetect();
	~FraudDetect();

    int init(int port,int backlog=1);
    int run();

    int sock_;
};

#endif
