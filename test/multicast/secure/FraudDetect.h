#ifndef __FRAUDDETECT_H__
#define __FRAUDDETECT_H__

#include "Classifier.h"

class FraudDetect{
public:
	FraudDetect();
	~FraudDetect();

    int init(int port,int backlog=0);
    int start(void *arg);
    static void* run(void *arg);
    static void verbosity(bool b);

    int msqid_;
    Classifier cf_;
protected:
    int sock_;
    static bool verbosity_;
};

#endif
