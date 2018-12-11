#include <iostream>
#include <unistd.h>
#include <stdio.h>

#define MAX 10000

pthread_mutex_t mtx;
int number=0;

void *add(void *arg)
{
    for(int i=0;i<MAX;i++) {
#ifdef LOCK
        pthread_mutex_lock(&mtx);
#endif
        number++;
        std::cout<<(char*)arg<<",number:"<<number<<std::endl;
#ifdef LOCK
        pthread_mutex_unlock(&mtx);
#endif
        usleep(0);
    }
    return 0;
}

void *del(void *arg)
{
    for(int i=0;i<MAX;i++) {
#ifdef LOCK
        pthread_mutex_lock(&mtx);
#endif
        number--;
        std::cout<<(char*)arg<<",number:"<<number<<std::endl;
#ifdef LOCK
        pthread_mutex_unlock(&mtx);
#endif
        usleep(1);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int rtn = pthread_mutex_init(&mtx,NULL);
    if(rtn) {
        std::cout<<"mutex init fail("<<rtn<<")"<<std::endl;
        return -1;
    }
    std::cout<<"mutex init ok"<<std::endl;

    pthread_t t[40];
    char* name[]={
                    "t01","t02","t03","t04","t05","t06","t07","t08","t09","t10",
                    "t11","t12","t13","t14","t15","t16","t17","t18","t19","t20",
                    "t21","t22","t23","t24","t25","t26","t27","t28","t29","t30",
                    "t31","t32","t33","t34","t35","t36","t37","t38","t39","t40",
    };

    void *(*runs[])(void *)={ add,del };
    for(int i=0;i<sizeof(t)/sizeof(pthread_t);i++){
        pthread_create(&t[i],NULL,runs[i%2],(void *)name[i]);
    }

    int status;
    for(int i=0;i<sizeof(t)/sizeof(pthread_t);i++){
        pthread_join(t[i],(void**)&status);
//      std::cout<<name[i]<<", join status="<<status<<std::endl;
    }

    pthread_mutex_destroy(&mtx);
    std::cout<<"end of main"<<std::endl;
    return 0;
}
