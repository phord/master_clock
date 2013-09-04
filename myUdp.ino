/*

 UDP interface for Arduino support

 Originally based on NtpUdp.c:
   created 4 Sep 2010
   by Michael Margolis
   modified 9 Apr 2012
   by Tom Igoe

   This code is in the public domain.

 */

#include <Arduino.h>
#include <IPAddress.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
//#include "myUdp.h"

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0x00, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress *timeServerAddress = NULL ;
unsigned int localPort = 8888;      // local port to listen for UDP packets
unsigned int serverPort = 0 ;

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
static bool active = false ;

void udpSetup(unsigned char* addr , unsigned int port )
{
  // start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    return ;
  }
  Udp.begin(localPort);
  timeServerAddress = new IPAddress( addr[0], addr[1], addr[2], addr[3]);
  serverPort = port ;
  active = true;
}

int sendUdp( char * data, int size )
{
  Udp.beginPacket(*timeServerAddress, serverPort);
  byte count = Udp.write((const unsigned char *)data,size);
  Udp.endPacket();
  return (int) count ;
}

int readUdp( char * buf, int size )
{
  int count = Udp.parsePacket() ;
  if ( count ) Udp.read( buf, size ) ;
  return count ;
}
