/*
   MasterClock - Drives a slave clock using the Internation Business Machine Time Protocols,
  Service Instructions No. 222, April 1, 1938,Form 231-8920
*/

//_____________________________________________________________________
//                                                             INCLUDES
#include "Arduino.h"
#include <stdarg.h>
#include "console.h"
#include "clock_generic.h"


//_____________________________________________________________________
//                                                           LOCAL VARS

static int m, s ;              ///< Current minutes and seconds
int a = LOW , b = LOW ;        ///< Desired A and B signal levels
int aForce = 0 ;               ///< Force A pulse by operator control
int bForce = 0 ;               ///< Force B pulse by operator control
static int tick = 0;           ///< The number of ticks since we started


typedef enum State {
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


//_____________________________________________________________________
// Increment the tick counter.  This is called once every 100ms by the
// hardware interrupt or main executive function.
void ticker() {
  ++tick;
}

//_____________________________________________________________________
// Tick accessors.  Because the 'tick' variable is modified during a
// hardware interrupt, it is not safe to read or write this variable
// outside of the interrupt routine unless we disable interrupts
// first.  These functions provide safe access to the tick variable.

//_____________________________________
// Advance the tick variable by 'adjust' ticks.
void setTick(int adjust) {
  noInterrupts() ;
  tick += adjust ;
  interrupts();
}

//_____________________________________
// Read the current tick variable.
int getTick() {
  noInterrupts() ;
  int x = tick ;
  interrupts();
  return x;
}

//_____________________________________
// Reset the tick counter to 0
void syncTime() {
  setTick( -getTick() ) ;
  state = rise ;
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
  consoleService() ;

  switch (state) {
  default:
  case rise:
    a = checkA() ;
    b = checkB() ;

    sendSignal( a , b ) ;        // Send output pulses (if any)

    showTime() ;                 // Report time and signals to serial port

    state = riseWait;
    break ;

  case riseWait:
    if ( getTick() < riseTime ) break ;
    setTick(-riseTime);
    state = fall;
    break;

  case fall:
    sendSignal( LOW , LOW ) ;     // End output pulses
    showSignalDrop() ;

    state = fallWait;
    break;

  case fallWait:
    if ( getTick() < fallTime ) break ;
    setTick(-fallTime);

    markTime() ;                 // Advance time

    state = rise;
    break;
  }
}
