#ifndef __DETECT_H__
#define __DETECT_H__

#include "SafeQueue.h"
#include "ISession.h"
typedef unsigned char byte;
typedef SafeQueue<std::vector<byte>> Queue;

class Detect : public IReceive{
public:
    Detect(){}
    virtual ~Detect(){}

    int start(void *arg) { return pthread_create(&tid_,NULL,run,arg); }
    int join(void **retval=0) { return pthread_join(tid_,retval); }
    ssize_t recv(void *buf,size_t len)
    {
        std::vector<byte> msg(len);
        memcpy(msg.data(),buf,len);
        q_.enqueue(msg);
        return len;
    }

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
