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

#ifdef LISTENER_DETECT
    const char* ip;
    int port;
    printf("Fraud Detecting activated\n");
#else
    printf("Fraud Detecting deactivated\n");
#endif

#ifdef LISTENER_DETECT
    if(argc>5) {
#else
    if(argc>3) {
#endif
        enable = (atoi(argv[1])==1)?1:0;
        printf("reuse socket address:");
        (enable)?printf("activated\n"):printf("deactivated\n");
        key = atoi(argv[2]);
        iv = atoi(argv[3]);
        printf("key=%d,iv=%d\n",key,iv);
#ifdef LISTENER_DETECT
        ip = argv[4];
        port = atoi(argv[5]);
        printf("fraud detector ip=%s,port=%d\n",ip,port);
#endif
    }else{
#ifdef LISTENER_DETECT
        printf("usage: listener <reuse> <key> <iv> <ip> <port>\n");
#else
        printf("usage: listener <reuse> <key> <iv>\n");
#endif
        printf("     reuse: don't reuse socket address(0),reuse socket address(1)\n");
        printf("     key,iv: any integer value, Arrays are filled with given integer recpectively\n");
#ifdef LISTENER_DETECT
        printf("     ip: fraud detector ip\n");
        printf("     port: fraud detector port\n");
#endif
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

    Collector *pCol = NULL;
#ifdef LISTENER_DETECT
    printf("Collector initilaizing...\n");
    pCol=new Collector();
    int errRtn;
    if((errRtn=pCol->init(ip,port))!=OK){
        delete pCol;
        pCol = NULL;
        printf("Collector init failed(%s)\n",errToMsg(errRtn));
    }else
        printf("Collector init successful(%s)\n",errToMsg(errRtn));
#endif

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

    unsigned int sequence;
    while (1) {
        socklen_t addrlen=sizeof(addr);
        if ((nbytes=recvfrom(fd,msgbuf,MSGBUFSIZE,0,
                            (struct sockaddr *)&addr,&addrlen)) < 0) {
            printf("recvfrom failed\n");
            delete pCol;
            return -1;
        }
        msgbuf[nbytes]=0;

        if((rtn=Bob.decrypt(d,tagSize,adata,std::string(msgbuf,nbytes),recoveredText))!=DECRYPT_OK) {
            printf("%s",Node::errToStr(rtn).c_str());
        }else{
            memcpy(&sequence,recoveredText.c_str(),sizeof(sequence));
            printf("sequence=%u ",sequence);
            if(pCol) pCol->collect(msgbuf,nbytes);
            printf("%s\n",recoveredText.c_str()+sizeof(sequence));
        }
        printf(" (%dbytes)\n",nbytes);
    }
}
