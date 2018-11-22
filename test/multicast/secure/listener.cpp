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

#define MULTICAST_GROUP "225.0.0.37"
#define MULTICAST_PORT 12345
#define MSGBUFSIZE 256

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int fd, nbytes;
    struct ip_mreq mreq;
    char msgbuf[MSGBUFSIZE];
    int enable=1;

    if(argc>1) {
        enable = (atoi(argv[1])==1)?1:0;
        printf("reuse socket address:");
        (enable)?printf("activated\n"):printf("deactivated\n");
    }else{
        printf("usage: listener <reuse>\n");
        printf("     reuse: don't reuse socket address(0),reuse socket address(1)\n");
        return -1;
    }

    if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
        perror("socket");
        return -1;
    }

    //allow multiple sockets to use the same PORT number 
    //if enable=1 then reuse, otherwise no reuse
    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&enable,sizeof(enable)) < 0) {
        perror("Reusing ADDR failed");
        return -1;
    }

    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(MULTICAST_PORT);
     
    if (bind(fd,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }
     
    mreq.imr_multiaddr.s_addr=inet_addr(MULTICAST_GROUP);
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    if (setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
        perror("setsockopt");
        return -1;
    }

    Node Bob;
    Bob.size_ = 32;
    Bob.key_ = new byte[Bob.size_];
    memset(Bob.key_,0,Bob.size_);
    memset(Bob.iv_,0,CryptoPP::AES::BLOCKSIZE);

    int tagSize = 16,rtn;
    CryptoPP::GCM<CryptoPP::AES>::Decryption d;
    d.SetKeyWithIV(Bob.key_,Bob.size_,Bob.iv_);
    std::string recoveredText;

    while (1) {
        socklen_t addrlen=sizeof(addr);
        if ((nbytes=recvfrom(fd,msgbuf,MSGBUFSIZE,0,
                            (struct sockaddr *)&addr,&addrlen)) < 0) {
              perror("recvfrom");
              return -1;
	     }
        msgbuf[nbytes]=0;
        puts(msgbuf);

        if((rtn=Bob.decrypt(d,tagSize,"AAD",msgbuf,recoveredText))!=DECRYPT_OK) {
            perror("decrypt error");
            return rtn;
        }
        printf("decrypted:");
        puts(recoveredText.c_str());
    }
}
