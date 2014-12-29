/*
    Master Clock - Drives an IBM Impulse Secondary clock movement
    using the International Business Machine Time Protocols,
    Service Instructions No. 230, April 1, 1938,Form 231-8884-0
    By Phil Hord,  This code is in the public domain Sept 9, 2013
*/

//_____________________________________________________________________
//                                                             INCLUDES
#include "Arduino.h"
#include <stdarg.h>
#include "console.h"
#include "clock_generic.h"
#include "Timer.h"
#include "NtpServer.h"
#include "Udp.h"

//_____________________________________________________________________
//                                                           LOCAL VARS

static int m, s ;              ///< Current minutes and seconds
int a = LOW , b = LOW ;        ///< Desired A and B signal levels
int aForce = 0 ;               ///< Force A pulse by operator control
int bForce = 0 ;               ///< Force B pulse by operator control

typedef enum State {
	reset ,                // Reset ticker to 0 to sync with exact second
	rise ,                 // Rising edge of pulse
	riseWait ,             // Wait for pulse-rise time
	fall ,                 // Falling edge of pulse
	fallWait ,             // Wait for pulse-fall time
} State ;

State state = rise ;           ///< Protocol chain state tracker.

//_____________________________________________________________________
//                                                            CONSTANTS

// Signal output duration, in 10ms ticks
enum {
	riseTime = 6 ,     // 600ms rise time
	fallTime = 4 ,     // 400ms fall time
} ;


//_____________________________________
// Reset the tick counter to 0
void syncTime() {
  state = reset ;
}

//_____________________________________________________________________
// Time accessors
// Let other functions get and set the clock time

int getMinutes() { return m; }
int getSeconds() { return s; }

void setMinutes( int minutes ) { m = minutes ; }
void setSeconds( int seconds ) { s = seconds ; }

void incMinutes( ) { if (++m > 59) m = 0 ; }
void incSeconds( ) { if (++s > 59) { s = 0 ; incMinutes() ; } }

void decMinutes( ) { if (--m < 0) m = 59 ; }
void decSeconds( ) { if (--s < 0) { s = 59 ; decMinutes() ; } }

//_____________________________________________________________________
// Signal accessors
// Let callers force A and B pulses

void sendPulseA() { ++aForce ; }
void sendPulseB() { ++bForce ; }

bool getA() { return a ; }        // Read the last A-signal level
bool getB() { return b ; }        // Read the last B-signal level

//_____________________________________________________________________
//                                                        TIME PROTOCOL
//
// These functions actually implement the time protocol.

//_____________________________________
// Called once per second.  Advances second and minute counters.
void markTime()
{
    incSeconds() ;
}

//_____________________________________
// Implement the A-signal protocol.
// Returns HIGH or LOW depending on what the A-signal output should
// based on the current time.
//
// The 'A' signal is raised once per minute at zero-seconds, and on
// every odd second between 10 and 50 during the 59th minute.
//
// Note: If 'aForce' is non-zero, return HIGH.
int checkA() {
    // Manual run based on serial input
    if ( aForce ) { aForce-- ; return HIGH ; }

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

//_____________________________________
// Implement the B-signal protocol.
// Returns HIGH or LOW depending on what the B-signal output should
// based on the current time.
//
// The 'B' signal is raised once per minute at zero-seconds for each
// minute except for minutes 50 to 59.
//
// Note: If 'bForce' is non-zero, return HIGH.
int checkB() {
    // Manual run based on serial input
    if ( bForce ) { bForce-- ; return HIGH ; }

    // pulse once per minute from 00 to 49
    if ( m > 49 ) return LOW ;

    if ( s == 0 ) return HIGH ;
    return LOW ;
}

//_____________________________________
// the service routine runs over and over again forever:
void service() {
  static int subTimer = 0;       ///< State counter per second

  consoleService() ;
  udpService( ) ;
  NtpService() ;

  switch (state) {
  default:
  case reset:
    subTimer = getTick() ;
  case rise:
    a = checkA() ;
    b = checkB() ;

    sendSignal( a , b ) ;        // Send output pulses (if any)

    showTime() ;                 // Report time and signals to serial port

    state = riseWait;
    break ;

  case riseWait:
    if ( elapsed(subTimer) < riseTime ) break ;
    subTimer += riseTime ;
    state = fall;
    break;

  case fall:
    sendSignal( LOW , LOW ) ;     // End output pulses
    showSignalDrop() ;

    state = fallWait;
    break;

  case fallWait:
    if ( elapsed(subTimer) < fallTime ) break ;
    subTimer += fallTime ;

    markTime() ;                 // Advance time

    state = rise;
    break;
  }
}
