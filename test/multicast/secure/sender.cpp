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
#include "FraudDetect.h"
#include "ErrorCode.h"
#include "message.h"
#include <sys/msg.h>

#define MULTICAST_GROUP "225.0.0.37"
#define MULTICAST_PORT 12345

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int fd, cnt,key,iv;
    struct ip_mreq mreq;
    char *message = NULL;

#ifdef PUBKEY_SECURITY
    printf("Publish key security activated\n");
#else
    printf("Publish key security is not activated\n");
#endif

    if(argc>3) {
        message = argv[1];
        printf("message:%s\n",message);
        key = atoi(argv[2]);
        iv = atoi(argv[3]);
        printf("key=%d,iv=%d\n",key,iv);
    }else{
        printf("usage: sender <message> <key> <iv>\n");
        printf("      key,iv: any integer value, Arrays are filled with given integer recpectively\n");
        return -1;
    }

    if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
        printf("fail to create socket\n");
        return -1;
    }

    struct message Msg;
    int msqid;
    if(-1==(msqid=msgget((key_t)1234,IPC_CREAT | 0666))) {
        printf("fail to get SystemV message queue\n");
        return -1;
    }
    printf("SystemV message queue:%d\n",msqid);

    FraudDetect fdetect;
    int errRtn;
    if((errRtn=fdetect.init(9191))!=OK)
        printf("FraudDetect init failed(%d)\n",errRtn);
    else{
        fdetect.start((void*)&msqid);
        printf("FraudDetect init successful(%d)\n",errRtn);
    }

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

#ifdef PUBKEY_SECURITY
	if(fopen("pubKey","r")==NULL) { //pubKey mocks actual key file
        printf("No publish key(pubKey),Unauthorized to use encryption module\n");
        return 0;
    }else{
        cipherText = Alice.encrypt(e,adata,message,tagSize);
        printf("Publish key confirmed\n");
    }
#else
    cipherText = Alice.encrypt(e,adata,message,tagSize);
#endif

    int i=0;
    ssize_t bytes;
    while (1) {
        if ((bytes=sendto(fd,cipherText.c_str(),cipherText.size(),0,(struct sockaddr *) &addr,sizeof(addr))) < 0) {
            printf("sendto failed\n");
            return -1;
        }
        Msg.type = 1;
        Msg.size = cipherText.size();
        memcpy(Msg.body,cipherText.c_str(),Msg.size);
        if(-1==msgsnd(msqid,(void *)&Msg,sizeof(Msg.body),IPC_NOWAIT)){
            printf("msgsnd fail\n");
            return -1;
        }
        printf("\r[%d] %zdbytes",++i,bytes);
        fflush(stdout);
        sleep(1);
    }
    return 0;
}
