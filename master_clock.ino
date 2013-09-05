/*
   MasterClock - Drives a slave clock using the International Business Machine Time Protocols,
  Service Instructions No. 222, April 1, 1938,Form 231-8920 
*/
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int ledA = 13;
int ledB = 12;

static int m, s ;
static int a = LOW , b = LOW ;

//Add 4 to lengthen time period. Clocl 3 minutes slow in 72 hours 7/10/2013, jf

#define second  1000     // Number of milliseconds in 1 second

// the setup routine runs once when you press reset:
void setup() {                
  Serial.begin(9600);
  // initialize the digital pin as an output.
  pinMode(ledA, OUTPUT);     
  pinMode(ledB, OUTPUT);     
}

#include <stdarg.h>
void p(char *fmt, ... ){
        char tmp[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(tmp, 128, fmt, args);
        va_end (args);
        Serial.print(tmp);
}

void markTime() {
  p("%02u:%02u:%02u %c%c\n", 0, m, s , (a?'A':' ') , (b?'B':' ') );
  ++s ;
  if ( s >59 ) {
    s = 0 ;
    ++m ;
  }
  if ( m > 59 ) m = 0;
}

int checkA() {
    // pulse once per minute
    if ( s == 0 ) return HIGH ;
    
    if ( m == 59 ) {
      if ( s >= 10 && s <= 50 ) {
        // On even seconds from 10 to 50, pulse A
        if ( ( s & 1 ) == 0 ) return HIGH ;
      }
    }
    return LOW ;
}

int checkB() {
    // pulse once per minute from 00 to 49
    if ( m > 49 ) return LOW ;
    
    if ( s == 0 ) return HIGH ;
    return LOW ;
}


// the loop routine runs over and over again forever:
void loop() {
  a = checkA() ;
  b = checkB() ;
  markTime();

  digitalWrite(ledA, a);   // Send A pulses
  digitalWrite(ledB, b);   // Send B pulses
  delay(second*0.6);               // wait for 600ms

  digitalWrite(ledA, LOW);    // End any A pulse
  digitalWrite(ledB, LOW);    // End any B pulse
  delay(second*0.4);               // wait out the second
}

