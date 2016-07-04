/*
    Master Clock - Drives an IBM Impulse Secondary clock movement
    using the International Business Machine Time Protocols,
    Service Instructions No. 230, April 1, 1938,Form 231-8884-0
    By Phil Hord,  This code is in the public domain Sept 9, 2013
 */

#define OAK

/* Library imports */
//#include <TimerOne.h>
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

/* Telnet server machine */
#include "TelnetServer.h"

/* NTP server machine */
#include "NtpServer.h"

// A and B signal pins
const int pulseA = 9;
const int pulseB = 8;
const int LED = 1;
const int RUN = 2;


int run_switch()
{
  return !digitalRead(RUN);
}

void sendSignal( int a, int b)
{
  digitalWrite(pulseA, a);   // Send A pulses
  digitalWrite(pulseB, b);   // Send B pulses
  digitalWrite(LED, a|b);    // BLink LED with any A or B pulse
}

void sendString( const char * str )
{
  #ifdef OAK
  Particle.print(str);
  #else
  Serial.print( str ) ;
  #endif
  TelnetWrite( str ) ;
}

char readKey()
{
  int key = TelnetRead() ;
  if ( key != -1 ) return (char) key ;

  #ifdef OAK
  return -1;    // TODO: Implement Particle.read()
  #else
  
  if (Serial.available() == 0)
    return -1 ;
  return (char) Serial.read();
  #endif
}

unsigned long us_per_tick = 100000 ;
void speedUp() {
  us_per_tick /= 2 ;
}

void slowDown() {
  us_per_tick *= 2 ;
}

// the setup routine runs once when you press reset:
void setup() {
  #ifdef OAK
  Particle.begin();
  Particle.println("Begin master_clock");
  #else
  Serial.begin(9600);
  #endif
  // initialize the digital pin as an output.
  pinMode(pulseA, OUTPUT);
  pinMode(pulseB, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(RUN, INPUT_PULLUP); // Use pullup mode to default HIGH
  NtpSetup() ;
  setupTelnetServer();
}

// the loop routine runs over and over again forever:
void loop() {
  service();
//  serviceTelnetServer();
}
