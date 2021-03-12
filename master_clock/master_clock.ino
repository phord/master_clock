/*
    Master Clock - Drives an IBM Impulse Secondary clock movement
    using the International Business Machine Time Protocols,
    Service Instructions No. 230, April 1, 1938,Form 231-8884-0
    By Phil Hord,  This code is in the public domain Sept 9, 2013
 */

/* Library imports */
#include <SPI.h>
//#include <EthernetDHCP.h>
//#include <EthernetUdp.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

/* Shared c++ code */
#include "clock_generic.h"

// Generic Network setup
#include "Network.h"

/* Telnet server machine */
#include "TelnetServer.h"

/* NTP server machine */
#include "NtpServer.h"

// Input/Output signal pins
const int pulseA = 14;
const int pulseB = 12;
const int pulseD = 13;
const int RUN = D3;


#include <TZ.h>
//#define MYTZ            TZ_America_Detroit              // Central time
#define MYTZ TZ_America_Los_Angeles

int run_switch()
{
  return !digitalRead(RUN);
}

void sendSignal( int a, int b, int d)
{
  digitalWrite(pulseA, a);   // Send A pulses
  digitalWrite(pulseB, b);   // Send B pulses
  digitalWrite(pulseD, d);   // Send D pulses
}

void sendString( const char * str )
{
  Serial.print( str ) ;
  TelnetWrite( str ) ;
}

char readKey()
{
  int key = TelnetRead() ;
  if ( key != -1 ) return (char) key ;

  if (Serial.available() == 0)
    return -1 ;
  return (char) Serial.read();
}

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);

  // Start ESP ntp time service
  configTime(MYTZ, "pool.ntp.org");

  // initialize the digital pin as an output.
  pinMode(pulseA, OUTPUT);
  pinMode(pulseB, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(RUN, INPUT_PULLUP); // Use pullup mode to default HIGH

  clockSetup();
}

// the loop routine runs over and over again forever:
void loop() {
  setupNetwork();
  service();
//  serviceTelnetServer();
}
