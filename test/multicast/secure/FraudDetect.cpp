#include "FraudDetect.h"
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "ErrorCode.h"
#include <pthread.h>
#include "message.h"
#include <sys/msg.h>

FraudDetect::FraudDetect()
{
}

FraudDetect::~FraudDetect()
{
    close(sock_);
}

int FraudDetect::init(int port,int backlog)
{
    if((sock_ = socket(AF_INET, SOCK_STREAM, 0))<0){
        return ERROR_SOCKET;
    }

    struct sockaddr_in addr;
    memset(&addr, 0x00, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if(bind(sock_, (struct sockaddr *)&addr, sizeof(addr)) < 0) return ERROR_BIND;
    if(listen(sock_,backlog) < 0) return ERROR_LISTEN;

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
    struct message msg;
    long msgtyp = 0;
    while((clientSock = accept(p->sock_, (struct sockaddr *)&clientAddr,&addrLen)) > 0){
        printf("clinet ip : %s\n", inet_ntoa(clientAddr.sin_addr));
        
        while(1) {
            if((rcvLen = recv(clientSock, buffer,sizeof(buffer), 0)) < 0){
                printf("error recv_len=%zd\n",rcvLen);
                break;
            }
            printf("<%zd>",rcvLen);
            while(1) {
                if(-1!=msgrcv(p->msqid_,(void*)&msg,sizeof(msg),msgtyp,MSG_NOERROR | IPC_NOWAIT)){
                    printf("+");
                }else{
                    printf(".\n");
                    break;
                }
            }
        }
        close(clientSock);
    }
    return 0;
}
