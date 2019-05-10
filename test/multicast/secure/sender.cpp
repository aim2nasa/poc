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
    int fd, cnt,key,iv;
    unsigned int usecs;
    struct ip_mreq mreq;
    char *message = NULL;

#ifdef PUBKEY_SECURITY
    printf("Publish key security activated\n");
#else
    printf("Publish key security is not activated\n");
#endif

#ifdef SENDER_DETECT
    printf("Fraud Detecting activated\n");
#else
    printf("Fraud Detecting deactivated\n");
#endif

    if(argc>4) {
        message = argv[1];
        printf("message:%s\n",message);
        key = atoi(argv[2]);
        iv = atoi(argv[3]);
        printf("key=%d,iv=%d\n",key,iv);
        usecs = atoi(argv[4]);
        printf("micro sleep:%dus(%fsec)\n",usecs,usecs/1000000.);
    }else{
        printf("usage: %s <message> <key> <iv> <usleep>\n",argv[0]);
        printf("      key,iv: any integer value, Arrays are filled with given integer recpectively\n");
        printf("      usleep: microsecond intervals\n");
        return -1;
    }

    if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
        printf("fail to create socket\n");
        return -1;
    }

#ifdef SENDER_DETECT
    struct message Msg;
    int msqid;
    if(-1==(msqid=msgget((key_t)1234,IPC_CREAT | 0666))) {
        printf("fail to get SystemV message queue\n");
        return -1;
    }
    printf("SystemV message queue:%d\n",msqid);
#endif

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

    unsigned int sequence=0;
    ssize_t bytes;
    char buffer[256];
    while (1) {
        sprintf(buffer,"%s-%u",message,sequence);
        bytes = strlen(buffer);
        memmove(buffer+sizeof(sequence),buffer,bytes);
        memcpy(buffer,&sequence,sizeof(sequence));
#ifdef PUBKEY_SECURITY
        FILE *fp;
        if((fp=fopen("pubKey","r"))==NULL) { //pubKey mocks actual key file
            printf("No publish key(pubKey),Unauthorized to use encryption module\n");
            return 0;
        }else{
            cipherText = Alice.encrypt(e,adata,reinterpret_cast<const byte*>(buffer),bytes+sizeof(sequence),tagSize);
        }
#else
        cipherText = Alice.encrypt(e,adata,reinterpret_cast<const byte*>(buffer),bytes+sizeof(sequence),tagSize);
#endif

#ifdef SENDER_DETECT
        Msg.type = 1;
        Msg.size = cipherText.size();
        memcpy(Msg.body,cipherText.c_str(),Msg.size);
        while(-1==msgsnd(msqid,(void *)&Msg,sizeof(Msg.body),IPC_NOWAIT)){
            struct message oldMsg;
            msgrcv(msqid,(void*)&oldMsg,sizeof(oldMsg),0,MSG_NOERROR | IPC_NOWAIT); //remove 1 from queue
        }
#endif
        if ((bytes=sendto(fd,cipherText.c_str(),cipherText.size(),0,(struct sockaddr *) &addr,sizeof(addr))) < 0) {
#ifdef PUBKEY_SECURITY
            fclose(fp);
#endif
            printf("sendto failed\n");
            return -1;
        }
        printf("[%u] %s (%zd) ",sequence++,buffer+sizeof(sequence),bytes);
        for(int j=0;j<cipherText.size();j++) printf("%x ",(unsigned char)cipherText.c_str()[j]);
        printf("\n");
        fflush(stdout);
        usleep(usecs);
#ifdef PUBKEY_SECURITY
        fclose(fp);
#endif
    }
    return 0;
}
