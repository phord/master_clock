#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <stdlib.h>
#include <arpa/inet.h>

int    server_sock;

void udpSetup(unsigned char* addr , unsigned int port ) {
  char strAddr[16] ;
  struct sockaddr_in to_addr;
  server_sock = socket (AF_INET,SOCK_DGRAM,0);
  if (server_sock == -1)
     fprintf(stderr,"Error in socket \n");

  sprintf(strAddr , "%u.%u.%u.%u", addr[0], addr[1], addr[2], addr[3] ) ;
  to_addr.sin_family      = AF_INET;
  to_addr.sin_port        = htons(port);
  to_addr.sin_addr.s_addr = inet_addr(strAddr);

  int flags;

  if (-1 == (flags = fcntl(server_sock, F_GETFL, 0))) flags = 0;
  if ( fcntl(server_sock, F_SETFL, flags | O_NONBLOCK) ) return ;

  if (connect(server_sock, (const struct sockaddr *) & to_addr, sizeof(to_addr)) == -1 )
     fprintf(stderr,"Error in connect \n");
        // close(sockfd);

}

int sendUdp( char * data, int size )
{
  return send(server_sock, data, size, 0) ;
}

int readUdp( char * buf, int size )
{
   int bytes = recv (server_sock, buf, size, 0);
   if ( bytes == -1 ) return 0;
   return bytes ;
}
