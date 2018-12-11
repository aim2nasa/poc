#include <iostream>

pthread_mutex_t mtx;

void *run(void *arg)
{
    std::cout<<"name:"<<(char*)arg<<std::endl;
}

int main(int argc, char *argv[])
{
    int rtn = pthread_mutex_init(&mtx,NULL);
    if(rtn) {
        std::cout<<"mutex init fail("<<rtn<<")"<<std::endl;
        return -1;
    }
    std::cout<<"mutex init ok"<<std::endl;

    pthread_t t[2];
    char *name[]={ (char*)"Thread1",(char*)"Thread2"};
    for(int i=0;i<sizeof(t)/sizeof(pthread_t);i++)
        pthread_create(&t[i],NULL,run,(void *)name[i]);

    int status;
    for(int i=0;i<sizeof(t)/sizeof(pthread_t);i++){
        pthread_join(t[i],(void**)&status);
        std::cout<<name[i]<<", join status="<<status<<std::endl;
    }

    pthread_mutex_destroy(&mtx);
    std::cout<<"end of main"<<std::endl;
    return 0;
}
