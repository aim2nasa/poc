#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define MULTICAST_GROUP "225.0.0.37"
#define MULTICAST_PORT 12345

int main(int argc, char *argv[])
{
     struct sockaddr_in addr;
     int fd, cnt;
     struct ip_mreq mreq;
     char *message="Hello, World!";

	  if(argc>1) {
	      message = argv[1];
	  }
	  printf("message:%s\n",message);

     if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
	      perror("socket");
	      return -1;
     }

     memset(&addr,0,sizeof(addr));
     addr.sin_family = AF_INET;
     addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
     addr.sin_port=htons(MULTICAST_PORT);
     
     while (1) {
	      if (sendto(fd,message,strlen(message),0,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
	     	    perror("sendto");
				 return -1;
	      }
	      sleep(1);
     }
	  return 0;
}
