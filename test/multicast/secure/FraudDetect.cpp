#include "FraudDetect.h"
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "ErrorCode.h"
#include <pthread.h>
#include <sys/msg.h>

bool FraudDetect::verbosity_(false);

FraudDetect::FraudDetect()
{
}

FraudDetect::~FraudDetect()
{
    close(sock_);
}

int FraudDetect::init(int port,int backlog)
{
    if((sock_ = socket(AF_INET, SOCK_DGRAM, 0))<0){
        return ERROR_SOCKET;
    }

    struct sockaddr_in addr;
    memset(&addr, 0x00, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if(bind(sock_, (struct sockaddr *)&addr, sizeof(addr)) < 0) return ERROR_BIND;
    return OK;
}

int FraudDetect::start(void *arg)
{
    pthread_t p_thread;
    return pthread_create(&p_thread,NULL,run,arg);
}

void* FraudDetect::run(void *arg)
{
    printf("\nwaiting for client..\n");
    FraudDetect* p = (FraudDetect*)arg;

    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    int clientSock;
    ssize_t rcvLen;
    char buffer[1024];
    struct messageCount msg;
    long msgtyp = 0;
    std::vector<messageCount> &q = p->cf_.q_;

    while(1) {
        if((rcvLen = recv(p->sock_, buffer,sizeof(buffer), 0)) < 0){
            printf("error recv_len=%zd\n",rcvLen);
            break;
        }
        if(rcvLen==0) break;
        printf("(%zd)",rcvLen);
        while(1) {
            if(-1!=msgrcv(p->msqid_,(void*)&msg,sizeof(message),msgtyp,MSG_NOERROR | IPC_NOWAIT)){
                while(q.size()>=MAX_QUEUE) { q.erase(q.begin()); printf("~"); }
                q.push_back(msg);
                printf("+");
                if(verbosity_) {
                    for(unsigned int j=0;j<msg.size;j++) printf("%x ",(unsigned char)msg.body[j]);
                    printf(",vc=%d ",msg.visitCount);
                }
            }else{
                printf("{%zd}",q.size());
                break;
            }
        }

        printf("-%d\n",p->cf_.ask(buffer,rcvLen));
    }
    return 0;
}

void FraudDetect::verbosity(bool b)
{
    verbosity_ = b;
}
