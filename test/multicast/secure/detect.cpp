#include <stdio.h>
#include <stdlib.h>
#include "FraudDetect.h"
#include "ErrorCode.h"
#include "message.h"

int main(int argc, char *argv[])
{
    int port,msqid;
    if(argc>2) {
        port = atoi(argv[1]);
        msqid = atoi(argv[2]);
        printf("port=%d,SystemV message queue:%d\n",port,msqid);
    }else{
        printf("usage: detect <port> <msqid>\n");
        printf("      port: data collecting port\n");
        printf("      msqid: SystemV message queue id\n");
        return -1;
    }

    FraudDetect fdetect;
    int errRtn;
    if((errRtn=fdetect.init(port))!=OK)
        printf("FraudDetect init failed(%s)\n",errToMsg(errRtn));
    else{
        fdetect.msqid_ = msqid;
        fdetect.run((void*)&fdetect);
    }
    return 0;
}
