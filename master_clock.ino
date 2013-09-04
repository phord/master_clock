/*
   MasterClock - Drives a slave clock using the International Business Machine Time Protocols,
  Service Instructions No. 222, April 1, 1938,Form 231-8920 
*/

#include "TimerOne.h"
#include "clock_generic.h"
//#include "clock_generic.c"


// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int ledA = 13;
int ledB = 12;

void sendSignal( int a, int b)
{
    digitalWrite(ledA, a);   // Send A pulses
    digitalWrite(ledB, b);   // Send B pulses
}

void sendString( const char * str )
{
   Serial.print( str ) ;
}

char readKey()
{
  if (Serial.available() == 0)
    return -1 ;
  return (char) Serial.read();
}

unsigned long us_per_tick = 100000 ;
void speedUp() {
  us_per_tick /= 2 ;
  Timer1.setPeriod(us_per_tick);    // Adjust tick speed
}

void slowDown() {
  us_per_tick *= 2 ;
  Timer1.setPeriod(us_per_tick);    // Adjust tick speed
}

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
  // initialize the digital pin as an output.
  pinMode(ledA, OUTPUT);
  pinMode(ledB, OUTPUT);
  Timer1.initialize(100000);         // initialize timer1 to 100ms period
  Timer1.attachInterrupt(ticker);    // attach timer overflow interrupt
  clockSetup() ;
}

// the loop routine runs over and over again forever:
void loop() {
  service();
}
