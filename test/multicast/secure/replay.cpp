#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <../../wc2/src/SafeQueue.h>

#define MULTICAST_GROUP "225.0.0.37"
#define MULTICAST_PORT 12345
#define MSGBUFSIZE 256

typedef unsigned char byte;
typedef SafeQueue<std::vector<byte>> Queue;

Queue q;
unsigned int usecs;

void* run(void *arg)
{
    printf("transmit thread started\n");
    int fd;
    if((fd=socket(AF_INET,SOCK_DGRAM,0))<0)
        perror("socket");

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(MULTICAST_GROUP);
    addr.sin_port=htons(MULTICAST_PORT);

    ssize_t sent;
    while(1){
        std::vector<byte> msg = q.dequeue();

        if((sent=sendto(fd,msg.data(),msg.size(),0,(struct sockaddr *) &addr,sizeof(addr))) < 0)
            perror("sendto");

        usleep(usecs);
        printf("r ");
        fflush(stdout);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int enable=1;
    if(argc>2) {
        enable = (atoi(argv[1])==1)?1:0;
        printf("reuse socket address:");
        (enable)?printf("activated\n"):printf("deactivated\n");
        usecs = atof(argv[2])*1000000;
        printf("micro sleep:%dus(%fsec)\n",usecs,usecs/1000000.);
    }else{
        printf("usage: replay <reuse> <sleep>\n");
        printf("     reuse: don't reuse socket address(0),reuse socket address(1)\n");
        printf("     sleep: second\n");
        return -1;
    }

    int fd;
    if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
        perror("socket");
        return -1;
    }

    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&enable,sizeof(enable)) < 0) {
        perror("Reusing ADDR failed");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(MULTICAST_PORT);
     
    if (bind(fd,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }
     
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr=inet_addr(MULTICAST_GROUP);
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    if (setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
        perror("setsockopt");
        return -1;
    }

    pthread_t p_thread;
    pthread_create(&p_thread,NULL,run,NULL);

    size_t nbytes;
    char msgbuf[MSGBUFSIZE];
    while (1) {
        socklen_t addrlen=sizeof(addr);
        if ((nbytes=recvfrom(fd,msgbuf,MSGBUFSIZE,0,
                            (struct sockaddr *)&addr,&addrlen)) < 0) {
              perror("recvfrom");
              return -1;
        }
        std::vector<byte> msg(nbytes);
        memcpy(msg.data(),msgbuf,nbytes);
        q.enqueue(msg);

        printf("(%zd)",nbytes);
        fflush(stdout);
    }
}