#ifndef __DETECT_H__
#define __DETECT_H__

#include "SafeQueue.h"
#include "ISession.h"
typedef unsigned char byte;
typedef SafeQueue<std::vector<byte>> Queue;

class Detect : public IReceive{
public:
    Detect():tid_(-1),stop_(false){}
    virtual ~Detect(){}

    int start(void *arg) { return pthread_create(&tid_,NULL,run,arg); }
    void stop()
    {
        stop_ = true;
        std::vector<byte> nullMsg;
        q_.enqueue(nullMsg);
    }
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
    bool stop_;
    static void* run(void *arg)
    {
        Detect *p = static_cast<Detect*>(arg);
        std::cout<<"start run.."<<std::endl;
        while(1)
        {
            std::vector<byte> msg = p->q_.dequeue();
            if(msg.size()==0 && p->stop_) break;    //Escape from infinite loop 

            std::cout<<msg.data()<<std::endl;
        }
        std::cout<<"end of run"<<std::endl;
        return 0;
    }
};

#endif
