#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip.h>

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

  if (connect(server_sock, (const struct sockaddr *) & to_addr, sizeof(to_addr)) == -1 )
     fprintf(stderr,"Error in connect \n");
        // close(sockfd);

}

int sendUdp( char * data, int size )
{
  return send(server_sock, data, size, 0) ;
}

void readUdp( char * buf, int size )
{
   int rc = recv (server_sock, buf, size, 0);
      printf("Received: %u bytes\n", rc);
}


const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
/*
 *
void main() {
  char packetBuffer[NTP_PACKET_SIZE];
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

	unsigned char addr[] = { 132, 163, 4, 101 } ;
	setupUdp(addr , 123 );

	sendUdp(packetBuffer, NTP_PACKET_SIZE);
	readUdp(packetBuffer, NTP_PACKET_SIZE) ;
}
*/
