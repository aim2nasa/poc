#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <entity/keyStore.h>
#include <entity/node.h>
#include "message.h"
#include <sys/msg.h>
#include <errno.h>

#define MULTICAST_GROUP "225.0.0.37"
#define MULTICAST_PORT 12345

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int fd, cnt,key,iv,msqid=-1;
    unsigned int frameNumber;
    struct ip_mreq mreq;
    char *message = NULL;

    if(argc>4) {
        frameNumber = atoi(argv[1]);
        printf("frameNumber:%u\n",frameNumber);
        message = argv[2];
        printf("message:%s\n",message);
        key = atoi(argv[3]);
        iv = atoi(argv[4]);
        printf("key=%d,iv=%d\n",key,iv);
        if(argc>5) {
            msqid = atoi(argv[5]);
            printf("msqid=%d\n",msqid);
        }
    }else{
        printf("usage: psender <frameNumber> <message> <key> <iv> <msqid>\n");
        printf("      frameNumber: frame number of the message\n");
        printf("      message: content user wants to send\n");
        printf("      key,iv: any integer value, Arrays are filled with given integer recpectively\n");
        printf("      msqid: linux message queu id (optional)\n");
        return -1;
    }

    if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
        printf("fail to create socket\n");
        return -1;
    }

    printf("SystemV message queue:%d\n",msqid);

    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
    addr.sin_port=htons(MULTICAST_PORT);
     
    Node Alice;
    Alice.size_= 32;
    Alice.key_ = new byte[Alice.size_];
    memset(Alice.key_,key,Alice.size_);
    memset(Alice.iv_,iv,CryptoPP::AES::BLOCKSIZE);

    int tagSize = 16;
    std::string adata(16, (char)0x00);
    CryptoPP::GCM<CryptoPP::AES>::Encryption e;
    e.SetKeyWithIV(Alice.key_,Alice.size_,Alice.iv_);
    std::string cipherText;

    ssize_t bytes;
    char buffer[256];

    sprintf(buffer,"%s-%u",message,frameNumber);
    bytes = strlen(buffer);
    printf("Message body:%s\n",buffer);
    memmove(buffer+sizeof(frameNumber),buffer,bytes);
    memcpy(buffer,&frameNumber,sizeof(frameNumber));
    printf("Full Message before encryption:");
    for(int j=0;j<sizeof(frameNumber)+bytes;j++) printf("%x ",(unsigned char)buffer[j]);
    printf("\n");

    cipherText = Alice.encrypt(e,adata,reinterpret_cast<const byte*>(buffer),bytes+sizeof(frameNumber),tagSize);

    printf("Encrypted Message:");
    for(int j=0;j<cipherText.size();j++) printf("%x ",(unsigned char)cipherText.c_str()[j]);
    printf("\n");

    if(msqid!=-1) {
        struct message Msg;
        Msg.type = 1;
        Msg.size = cipherText.size();
        memcpy(Msg.body,cipherText.c_str(),Msg.size);
        while(-1==msgsnd(msqid,(void *)&Msg,sizeof(Msg.body),IPC_NOWAIT)){
            struct message oldMsg;
            msgrcv(msqid,(void*)&oldMsg,sizeof(oldMsg),0,MSG_NOERROR | IPC_NOWAIT); //remove 1 from queue
        }
    }

    if ((bytes=sendto(fd,cipherText.c_str(),cipherText.size(),0,(struct sockaddr *) &addr,sizeof(addr))) < 0) {
        printf("sendto failed\n");
        return -1;
    }

    printf("[%u] %s (%zd)\n",frameNumber,buffer+sizeof(frameNumber),bytes);
    return 0;
}
