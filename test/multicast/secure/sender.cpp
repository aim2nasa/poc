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

#define MULTICAST_GROUP "225.0.0.37"
#define MULTICAST_PORT 12345

int main(int argc, char *argv[])
{
     struct sockaddr_in addr;
     int fd, cnt;
     struct ip_mreq mreq;
     char *message = NULL;

     if(argc>1) {
         message = argv[1];
         printf("message:%s\n",message);
     }else{
         printf("usage: sender <message>\n");
         return -1;
     }

     if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
         perror("socket");
         return -1;
     }

     memset(&addr,0,sizeof(addr));
     addr.sin_family = AF_INET;
     addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
     addr.sin_port=htons(MULTICAST_PORT);
     
     Node Alice;
     Alice.size_= 32;
     Alice.key_ = new byte[Alice.size_];
     memset(Alice.key_,0,Alice.size_);
     memset(Alice.iv_,0,CryptoPP::AES::BLOCKSIZE);

     int tagSize = 16;
     CryptoPP::GCM<CryptoPP::AES>::Encryption e;
     e.SetKeyWithIV(Alice.key_,Alice.size_,Alice.iv_);
     std::string cipherText = Alice.encrypt(e,"AAD",message,tagSize);

     while (1) {
         if (sendto(fd,cipherText.c_str(),cipherText.size(),0,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
             perror("sendto");
             return -1;
         }
         sleep(1);
     }
     return 0;
}
