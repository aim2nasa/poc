#include <iostream>

pthread_mutex_t mtx;

int main(int argc, char *argv[])
{
    int rtn = pthread_mutex_init(&mtx,NULL);
    if(rtn) {
        std::cout<<"mutex init fail("<<rtn<<")"<<std::endl;
        return -1;
    }
    std::cout<<"mutex init ok"<<std::endl;

    pthread_mutex_destroy(&mtx);
    std::cout<<"end of main"<<std::endl;
    return 0;
}
