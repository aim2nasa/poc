#include "FraudDetect.h"
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "ErrorCode.h"
#include <pthread.h>
#include <sys/msg.h>

FraudDetect::FraudDetect()
{
}

FraudDetect::~FraudDetect()
{
    close(sock_);
}

int FraudDetect::init(int port,int backlog)
{
    if((sock_ = socket(AF_INET, SOCK_DGRAM, 0))<0){
        return ERROR_SOCKET;
    }

    struct sockaddr_in addr;
    memset(&addr, 0x00, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if(bind(sock_, (struct sockaddr *)&addr, sizeof(addr)) < 0) return ERROR_BIND;
    return OK;
}

int FraudDetect::start(void *arg)
{
    pthread_t p_thread;
    return pthread_create(&p_thread,NULL,run,arg);
}

bool FraudDetect::exist(std::vector<messageCount>& q,const char *buff,unsigned int buffSize)
{
    for(std::vector<messageCount>::iterator it = q.begin(); it != q.end(); ++it){
        if(memcmp((*it).body,buff,buffSize)==0) return true;
    }
    return false;
}

int FraudDetect::existOrder(std::vector<messageCount>& q,const char *buff,unsigned int buffSize)
{
    int i=1;
    for(std::vector<messageCount>::iterator it = q.begin(); it != q.end(); ++it){
        if(memcmp((*it).body,buff,buffSize)==0) return i;
        i++;
    }
    return -1;
}

void* FraudDetect::run(void *arg)
{
    printf("\nwaiting for client..\n");
    FraudDetect* p = (FraudDetect*)arg;

    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    int clientSock;
    ssize_t rcvLen;
    char buffer[1024];
    struct messageCount msg;
    long msgtyp = 0;
    std::vector<messageCount> q;

    int tagSize = 16;
    CryptoPP::GCM<CryptoPP::AES>::Decryption d;
    d.SetKeyWithIV(p->Bob_.key_,p->Bob_.size_,p->Bob_.iv_);
    std::string recoveredText;
    std::string adata(16, (char)0x00);

    int sequence;
    while(1) {
        if((rcvLen = recv(p->sock_, buffer,sizeof(buffer), 0)) < 0){
            printf("error recv_len=%zd\n",rcvLen);
            break;
        }
        if(rcvLen==0) break;
        printf("<%zd>",rcvLen);
        while(1) {
            if(-1!=msgrcv(p->msqid_,(void*)&msg,sizeof(message),msgtyp,MSG_NOERROR | IPC_NOWAIT)){
                while(q.size()>=MAX_QUEUE) { q.erase(q.begin()); printf("~"); }
                q.push_back(msg);
                for(unsigned int j=0;j<msg.size;j++) printf("%x ",(unsigned char)msg.body[j]);
                printf(",vc=%d ",msg.visitCount);
                printf("+");
            }else{
                printf("(%zd).",q.size());
                break;
            }
        }

        if(p->Bob_.size_>0) {
            int rtn;
            if((rtn=p->Bob_.decrypt(d,tagSize,adata,std::string(buffer,rcvLen),recoveredText))!=DECRYPT_OK){
                printf(" %s",Node::errToStr(rtn).c_str());
            }else{
                memcpy(&sequence,recoveredText.c_str(),sizeof(int));
                printf("%u %s",sequence,recoveredText.c_str()+sizeof(int));
            }
        }

        int order;
        ((order=existOrder(q,buffer,rcvLen))>0)?printf(" [O]"):printf(" [X]");
        printf(" %d/%zd\n",order,q.size());
        fflush(stdout);
    }
    return 0;
}

void FraudDetect::setKeys(int size,int key,int iv)
{
    Bob_.size_ = size;
    Bob_.key_ = new byte[Bob_.size_];
    memset(Bob_.key_,key,Bob_.size_);
    memset(Bob_.iv_,iv,CryptoPP::AES::BLOCKSIZE);
}
