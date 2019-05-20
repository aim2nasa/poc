#include <stdio.h>
#include <stdlib.h>
#include "FraudDetect.h"
#include "ErrorCode.h"
#include "message.h"
#include <cryptopp/aes.h>

int main(int argc, char *argv[])
{
    int port,msqid,key,iv;
    if(argc>2) {
        port = atoi(argv[1]);
        msqid = atoi(argv[2]);
        printf("port=%d,SystemV message queue:%d\n",port,msqid);
        if(argc>4) {
            key = atoi(argv[3]);
            iv = atoi(argv[4]);
            printf("key=%d,iv=%d\n",key,iv);
        }
    }else{
        printf("usage: detect <port> <msqid> <key> <iv>\n");
        printf("      port: data collecting port\n");
        printf("      msqid: SystemV message queue id\n");
        printf("      key,iv (optional): any integer value, Arrays are filled with given integer recpectively\n");
        return -1;
    }

    FraudDetect fdetect;
    int errRtn;
    if((errRtn=fdetect.init(port))!=OK)
        printf("FraudDetect init failed(%s)\n",errToMsg(errRtn));
    else{
        fdetect.msqid_ = msqid;
        if(argc>4) {
            std::string adata(16, (char)0x00);
            fdetect.cf_.init(16,adata,32,key,iv);
        }
        fdetect.run((void*)&fdetect);
    }
    return 0;
}
