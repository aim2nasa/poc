#include "FraudDetect.h"
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

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

int FraudDetect::run()
{
    int clientSock;
    struct sockaddr_in clientAddr;
    char buffer[1024];

    socklen_t addrLen = sizeof(clientAddr);
    while((clientSock = accept(sock_, (struct sockaddr *)&clientAddr, &addrLen)) > 0){
        printf("clinet ip : %s\n", inet_ntoa(clientAddr.sin_addr));
    }
    close(clientSock);
    return 0;
}
