#ifndef __DETECT_H__
#define __DETECT_H__

class Detect{
public:
    Detect(){}
    virtual ~Detect(){}

    int start(void *arg);
    int join(void **retval=0);

private:
    pthread_t tid_;
    static void* run(void *arg);
};

int Detect::start(void *arg)
{
    return pthread_create(&tid_,NULL,run,arg);
}

int Detect::join(void **retval)
{
    return pthread_join(tid_,retval);
}

void* Detect::run(void *arg)
{
    std::cout<<"start run.."<<std::endl;
    std::cout<<"end of run"<<std::endl;
    return 0;
}

#endif
