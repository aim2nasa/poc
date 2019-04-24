#ifndef __DETECT_H__
#define __DETECT_H__

#include "SafeQueue.h"
typedef unsigned char byte;
typedef SafeQueue<std::vector<byte>> Queue;

class Detect{
public:
    Detect(){}
    virtual ~Detect(){}

    int start(void *arg) { return pthread_create(&tid_,NULL,run,arg); }
    int join(void **retval=0) { return pthread_join(tid_,retval); }

private:
    pthread_t tid_;
    Queue q_;
    static void* run(void *arg)
    {
        Detect *p = static_cast<Detect*>(arg);
        std::cout<<"start run.."<<std::endl;
        std::cout<<"end of run"<<std::endl;
        return 0;
    }
};

#endif
