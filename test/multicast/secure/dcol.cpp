#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <entity/keyStore.h>
#include <entity/node.h>
#include "Collector.h"
#include "ErrorCode.h"

#define MULTICAST_GROUP "225.0.0.37"
#define MULTICAST_PORT 12345
#define MSGBUFSIZE 256

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int fd, nbytes,key,iv;
    struct ip_mreq mreq;
    char msgbuf[MSGBUFSIZE];
    int enable=1;

    const char* ip;
    int port;
    printf("Data Collector\n");

    if(argc>3) {
        enable = (atoi(argv[1])==1)?1:0;
        printf("reuse socket address:");
        (enable)?printf("activated\n"):printf("deactivated\n");
        printf("key=%d,iv=%d\n",key,iv);
        ip = argv[2];
        port = atoi(argv[3]);
        printf("Detector ip=%s,port=%d\n",ip,port);
    }else{
        printf("usage: dcol <reuse> <ip> <port>\n");
        printf("     reuse: don't reuse socket address(0),reuse socket address(1)\n");
        printf("     ip: Detector ip\n");
        printf("     port: Detector port\n");
        return -1;
    }

    if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
        printf("fail to create socket\n");
        return -1;
    }

    //allow multiple sockets to use the same PORT number 
    //if enable=1 then reuse, otherwise no reuse
    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&enable,sizeof(enable)) < 0) {
        printf("Reusing ADDR failed\n");
        return -1;
    }

    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(MULTICAST_PORT);
     
    if (bind(fd,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
        printf("bind failed\n");
        return -1;
    }
     
    mreq.imr_multiaddr.s_addr=inet_addr(MULTICAST_GROUP);
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    if (setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
        printf("setsockopt failed\n");
        return -1;
    }

    printf("Collector initilaizing...\n");
    Collector *pCol=new Collector();
    int errRtn;
    if((errRtn=pCol->init(ip,port))!=OK){
        delete pCol;
        pCol = NULL;
        printf("Collector init failed(%s)\n",errToMsg(errRtn));
    }else
        printf("Collector init successful(%s)\n",errToMsg(errRtn));

    while (1) {
        socklen_t addrlen=sizeof(addr);
        if ((nbytes=recvfrom(fd,msgbuf,MSGBUFSIZE,0,
                            (struct sockaddr *)&addr,&addrlen)) < 0) {
            printf("recvfrom failed\n");
            delete pCol;
            return -1;
        }
        msgbuf[nbytes]=0;

        pCol->collect(msgbuf,nbytes);
        printf("(%d)\n",nbytes);
    }
}
