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
//#include <EthernetDHCP.h>
//#include <EthernetUdp.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include "console.h"
#include "ConfigData.h"

IPAddress timeServerAddress ;
unsigned int localPort = 8888;      // local port to listen for UDP packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

static bool active = false ;
const char* ntpServerName = "time.nist.gov";

void reportMac()
{
}

// Just a utility function to nicely format an IP address.
const char* ip_to_str(const uint8_t* ipAddr)
{
  static char buf[16];
  sprintf(buf, "%d.%d.%d.%d", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  return buf;
}

void udpSetup(unsigned char* addr , unsigned int port )
{
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("Starting UDP");
  Udp.begin(port);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerAddress );
  //timeServerAddress = new IPAddress( addr[0], addr[1], addr[2], addr[3]);
}

void udpService( )
{
  if ( ! active ) {
    p("Starting NTP handler on port %u\n", localPort ) ;
    Udp.begin(localPort);
    active = true;
  }
}

bool udpActive () { return !!active ; }

int sendUdp( char * data, int size )
{
  if ( ! active ) return 0 ;

  Udp.beginPacket(timeServerAddress, 123); //NTP requests are to port 123
  byte count = Udp.write(data, size);
  Udp.endPacket();

  return (int) count ;
}

int readUdp( char * buf, int size )
{
  if ( ! active ) return 0 ;

  int count = Udp.parsePacket();
  if (count) {
    Serial.print("packet received, length=");
    Serial.println(count);
    Udp.read(buf, size);
  }
  return count ;
}
