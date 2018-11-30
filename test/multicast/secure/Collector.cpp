#include "Collector.h"
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "ErrorCode.h"

Collector::Collector()
{
}

Collector::~Collector()
{
    close(sock_);
}

int Collector::init(const char* ip,int port)
{
    if((sock_ = socket(AF_INET, SOCK_STREAM, 0))<0) ERROR_SOCKET;

    struct sockaddr_in addr;

    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

    if(connect(sock_, (struct sockaddr *)&addr, sizeof(addr)) < 0) return ERROR_CONNECT;
    return OK;
}
