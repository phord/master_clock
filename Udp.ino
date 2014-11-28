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
#include <EthernetDHCP.h>
#include <EthernetUdp.h>
#include "console.h"
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

void reportMac()
{
  p("Programmed MAC address=%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] ) ;
}

// Just a utility function to nicely format an IP address.
const char* ip_to_str(const uint8_t* ipAddr)
{
  static char buf[16];
  sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  return buf;
}

void udpSetup(unsigned char* addr , unsigned int port )
{
  // start Ethernet and UDP
  EthernetDHCP.begin(mac, 1);
  timeServerAddress = new IPAddress( addr[0], addr[1], addr[2], addr[3]);
  serverPort = port ;
}

void udpService( )
{
  static DhcpState prevState = DhcpStateNone;
  static unsigned long prevTime = 0;
  
  // poll() queries the DHCP library for its current state (all possible values
  // are shown in the switch statement below). This way, you can find out if a
  // lease has been obtained or is in the process of being renewed, without
  // blocking your sketch. Therefore, you could display an error message or
  // something if a lease cannot be obtained within reasonable time.
  // Also, poll() will actually run the DHCP module, just like maintain(), so
  // you should call either of these two methods at least once within your
  // loop() section, or you risk losing your DHCP lease when it expires!
  DhcpState state = EthernetDHCP.poll();

  if (prevState != state) {

    switch (state) {
      case DhcpStateDiscovering:
        p("DHCP Discover\n");
        break;
      case DhcpStateRequesting:
        Serial.print("DHCP Request\n");
        break;
      case DhcpStateRenewing:
        Serial.print("DHCP Renew\n");
        break;
      case DhcpStateLeased: {
        Serial.println("DHCP Obtained\n");

        // Since we're here, it means that we now have a DHCP lease, so we
        // print out some information.
        const byte* ipAddr = EthernetDHCP.ipAddress();
        const byte* gatewayAddr = EthernetDHCP.gatewayIpAddress();
        const byte* dnsAddr = EthernetDHCP.dnsIpAddress();

        p(" IP address: %s\n", ip_to_str(ipAddr));
        p("    Gateway: %s\n", ip_to_str(gatewayAddr));
        p("        DNS: %s\n", ip_to_str(dnsAddr));

        if ( ! active ) {
          p("Starting NTP handler on port %u\n", localPort ) ;
          Udp.begin(localPort);
          active = true;
        }
        break;
      }
    }
    prevState = state;
  }
}

bool udpActive () { return !!active ; }

int sendUdp( char * data, int size )
{
  if ( ! active ) return 0 ;
  
  Udp.beginPacket(*timeServerAddress, serverPort);
  byte count = Udp.write((const unsigned char *)data,size);
  Udp.endPacket();
  return (int) count ;
}

int readUdp( char * buf, int size )
{
  if ( ! active ) return 0 ;
  int count = Udp.parsePacket() ;
  if ( count ) Udp.read( buf, size ) ;
  return count ;
}
