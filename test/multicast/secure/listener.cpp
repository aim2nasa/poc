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

    if(argc>3) {
        enable = (atoi(argv[1])==1)?1:0;
        printf("reuse socket address:");
        (enable)?printf("activated\n"):printf("deactivated\n");
        key = atoi(argv[2]);
        iv = atoi(argv[3]);
        printf("key=%d,iv=%d\n",key,iv);
    }else{
        printf("usage: listener <reuse> <key> <iv>\n");
        printf("     reuse: don't reuse socket address(0),reuse socket address(1)\n");
        printf("     key,iv: any integer value, Arrays are filled with given integer recpectively\n");
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
    Collector col;
    int errRtn;
    if((errRtn=col.init("127.0.0.1",9191))!=OK)
        printf("Collector init failed(%d)\n",errRtn);
    else
        printf("Collector init successful(%d)\n",errRtn);

    Node Bob;
    Bob.size_ = 32;
    Bob.key_ = new byte[Bob.size_];
    memset(Bob.key_,key,Bob.size_);
    memset(Bob.iv_,iv,CryptoPP::AES::BLOCKSIZE);

    int tagSize = 16,rtn;
    CryptoPP::GCM<CryptoPP::AES>::Decryption d;
    d.SetKeyWithIV(Bob.key_,Bob.size_,Bob.iv_);
    std::string recoveredText;
    std::string adata(16, (char)0x00);

    while (1) {
        socklen_t addrlen=sizeof(addr);
        if ((nbytes=recvfrom(fd,msgbuf,MSGBUFSIZE,0,
                            (struct sockaddr *)&addr,&addrlen)) < 0) {
              printf("recvfrom failed\n");
              return -1;
	     }
        msgbuf[nbytes]=0;

        printf("[%d]",nbytes);
        if((rtn=Bob.decrypt(d,tagSize,adata,msgbuf,recoveredText))!=DECRYPT_OK) {
            printf("%s\n",Node::errToStr(rtn).c_str());
        }else{
            puts(recoveredText.c_str());
        }
    }
}
