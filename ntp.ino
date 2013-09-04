/*

 Udp NTP Client

 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket
 For more on NTP time servers and the messages needed to communicate with them,
 see http://en.wikipedia.org/wiki/Network_Time_Protocol

 Warning: NTP Servers are subject to temporary failure or IP address change.
 Plese check

    http://tf.nist.gov/tf-cgi/servers.cgi

 if the time server used in the example didn't work.

 created 4 Sep 2010
 by Michael Margolis
 modified 9 Apr 2012
 by Tom Igoe

 This code is in the public domain.

 */

#include "Udp.h"
#include "console.h"

unsigned char timeServer[] = {132, 163, 4, 101}; // time-a.timefreq.bldrdoc.gov NTP server
// IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov NTP server
// IPAddress timeServer(132, 163, 4, 103); // time-c.timefreq.bldrdoc.gov NTP server

const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message

unsigned char packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

void ntpSetup() {
	udpSetup( timeServer , 123 ) ;
}

bool readNtpResponse( int *minutes , int * seconds ) {
  int readBytes = readUdp( (char *)packetBuffer, sizeof(packetBuffer) ) ;
  if ( ! readBytes ) return false ;

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = (packetBuffer[40]<<8 ) + packetBuffer[41];
    unsigned long lowWord  = (packetBuffer[42]<<8 ) + packetBuffer[43];
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;

    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:

    int hours = (epoch  % 86400L) / 3600 ;
    *minutes  = (epoch  % 3600) / 60;
    *seconds  = epoch % 60;

    p("NTP time = %02u:%02u:%02u UTC\n", hours , *minutes, *seconds ); // print the time

    return true ;
}

// send an NTP request to the time server at the given address
void sendNtpRequest()
{
  // set all bytes in the buffer to 0
  for (int i = 0 ; i < NTP_PACKET_SIZE ; i++ ) packetBuffer[i]= 0;

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

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  sendUdp((char *)packetBuffer , NTP_PACKET_SIZE ) ;
}
