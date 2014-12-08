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
#include "ntp.h"
#include "Udp.h"

//_____________________________________________________________________
//                                                           LOCAL VARS

static int m, s ;              ///< Current minutes and seconds
int a = LOW , b = LOW ;        ///< Desired A and B signal levels
int aForce = 0 ;               ///< Force A pulse by operator control
int bForce = 0 ;               ///< Force B pulse by operator control
static int realTick = 0;       ///< The number of ticks since we started


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


//_____________________________________________________________________
// Increment the tick counter.  This is called once every 100ms by the
// hardware interrupt or main executive function.
void ticker() {
  ++realTick;
}

//_____________________________________________________________________
// Tick accessor.  Because the 'tick' variable is modified during a
// hardware interrupt, it is not safe to read or write this variable
// outside of the interrupt routine unless we disable interrupts
// first.  These functions provide safe access to the tick variable.

//_____________________________________
// Read the current tick variable.
int getTick() {
  noInterrupts() ;
  int x = realTick ;
  interrupts();
  return x;
}

//_____________________________________
// Read number of elapsed ticks relative to timer variable
int elapsed( int timer ) {
  return getTick() - timer ;
}

//_____________________________________
// Return a timer tick from some point in the future
int getFuture( int nSeconds ) {
  return getTick() + nSeconds * 10 ;
}

//_____________________________________
// Test if a timer has expired (is in the past)
bool expired( int timer ) {
  return elapsed(timer) >= 0;
}

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

void setTimeFromNtp( unsigned long now ) {
  static unsigned long firstNtp = 0 ;    ///< First sync time
  static long totalDrift = 0;            ///< Total seconds lost or gained

  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years to show epoch time
  unsigned long epoch = now - seventyYears;

  // print Unix time:
  p("Unix time = %lu\n", epoch);

  unsigned hh = (now % 86400L) / 3600 ;
  unsigned mm = (now  % 3600) / 60;
  unsigned ss = now % 60;

  p("NTP time = %02u:%02u:%02u UTC\n", hh , mm , ss ); // print the time

  long delta = (now % 3600L ) - (m*60+s) ;
  p("\nNTP Server: adjusting time by %d seconds.\n" ,  delta ) ;

  //-- Accumulate error history
  if (firstNtp) {
    totalDrift += delta ;

    //-- Report error history
    p("Seconds since prev NTP: %ld   Total delta:  %ld\n" , now - firstNtp, totalDrift ) ;
  }

  setSeconds(ss) ;
  setMinutes(mm) ;

  //-- First known time sync
  if (!firstNtp) firstNtp = now ;
}
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

typedef enum Ntp_State {
    ntp_idle ,   	///< NTP is waiting to be triggered
    ntp_cron ,          ///< Auto-trigger NTP request
    ntp_request , 	///< Sending NTP request
    ntp_response ,      ///< Waiting for NTP response
    ntp_completed ,     ///< NTP completed; waiting for reset
    ntp_oneshot ,       ///< Try a short NTP request (manual intervention)
} Ntp_State ;

static Ntp_State ntpState = ntp_idle ;

void triggerNtp() {
	ntpState = ntp_oneshot ;
}

void checkNtp() {
    static int ntpResponseTimeout = 0 ;
    static int ntpRequestGuardTime = 0 ;
    static int retries ;

//	p("\r%d:  %d  %d  %d    ", ntpState , realTick, ntpResponseTimeout , retries ) ;
    switch ( ntpState ) {
    case ntp_idle :
        if ( m == 58 ) ntpState = ntp_cron ;
        break ;

    case ntp_cron:
	ntpRequestGuardTime = getFuture( 15 * 60 ) ;    // Prevent multiple automatic requests within 15 minutes
        retries = 30 ;
        ntpState = ntp_request ;
        break ;

    case ntp_oneshot:
        retries = 4 ;
        ntpState = ntp_request ;
        break ;

    case ntp_request :
        if ( ! udpActive() ) break ;
        sendNtpRequest() ;
	ntpResponseTimeout = getFuture( 10 ) ;  	// timeout if we don't get a response in 10 seconds
	ntpState = ntp_response ;
        break ;

    case ntp_response :
        if ( expired(ntpResponseTimeout) ) {
            ntpState = ntp_request ;
	    if ( --retries < 0 ) ntpState = ntp_completed ;
        }
	else
	{
           unsigned long ntpTime ;
	    bool success = readNtpResponse( ntpTime ) ;
	    if ( success ) {
                setTimeFromNtp(ntpTime);
	        ntpState = ntp_completed ;
	    }
	}
        break ;

    case ntp_completed :
        if ( expired(ntpRequestGuardTime) ) {
	  ntpState = ntp_idle ;
        }
        break ;
    }
}

void clockSetup() {
	ntpSetup() ;
	triggerNtp() ;
}

//_____________________________________
// the service routine runs over and over again forever:
void service() {
  static int subTimer = 0;       ///< State counter per second
  consoleService() ;
  udpService( ) ;

  checkNtp() ;

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
