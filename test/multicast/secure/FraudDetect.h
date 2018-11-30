#ifndef __FRAUDDETECT_H__
#define __FRAUDDETECT_H__

#define OK              0
#define ERROR_SOCKET    -10
#define ERROR_BIND      -10
#define ERROR_LISTEN    -10

class FraudDetect{
public:
	FraudDetect();
	~FraudDetect();

    int init(int port,int backlog=1);

    int sock_;
};

#endif
