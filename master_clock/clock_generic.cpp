/*
    Master Clock - Drives an IBM Impulse Secondary clock movement
    using the International Business Machine Time Protocols,
    Service Instructions No. 230, April 1, 1938,Form 231-8884-0
    By Phil Hord,  This code is in the public domain Sept 9, 2013
*/

//_____________________________________________________________________
//                                                             INCLUDES
#include <time.h>
#include "Arduino.h"
#include <stdarg.h>
#include "console.h"
#include "clock_generic.h"
#include "Timer.h"
#include "NtpServer.h"
#include "TimeSave.h"
#include "TimeService.h"

//_____________________________________________________________________
//                                                           LOCAL VARS

static unsigned walltime ;     ///< Current hours/minutes displayed on clock
int a = LOW , b = LOW ;        ///< Desired A and B signal levels
int d = LOW ;                  ///< Desired D signal level
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

//_____________________________________________________________________
// Time accessors
// Let other functions get and set the clock time

// Get real time from system in seconds
int getRealTime() {
        time_t now = TimeService::localtime();
        return now % MAX_TIME;
}

// Get time displayed on clock in seconds
int getWallTime() { return walltime * 60; }

// Set time displayed on clock in seconds
void setWallTime(int seconds) {
        walltime = (seconds % MAX_TIME) / 60;
}

// Advance the time on the wall display clock
void incMinutes() {
        walltime++;
        walltime %= MAX_TIME / 60;
}

// Set time displayed on clock to real time
void resetWallTime() {
        setWallTime(TimeService::localtime());
}

//_____________________________________________________________________
// Signal accessors
// Let callers force A and B pulses

void sendPulseA() { ++aForce ; }
void sendPulseB() { ++bForce ; }

bool getA() { return a ; }        // Read the last A-signal level
bool getB() { return b ; }        // Read the last B-signal level
bool getD() { return d ; }        // Read the last D-signal level

//_____________________________________________________________________
//                                                        TIME PROTOCOL
//
// These functions actually implement the time protocol.

//_____________________________________
// Implement the A-signal protocol.
// Returns HIGH or LOW depending on what the A-signal output should
// based on the current time.
//
// The 'A' signal is raised once per minute at zero-seconds, and on
// every odd second between 10 and 50 during the 59th minute.
//
// Note: If 'aForce' is non-zero or pin D2 is high, return HIGH.
int checkA(unsigned t) {
    // Manual run based on serial input
    if ( aForce ) { aForce-- ; return HIGH ; }

    unsigned int s = t % 60;
    unsigned int m = (t / 60) % 60;

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
int checkB(unsigned t) {
    // Manual run based on serial input
    if ( bForce ) { bForce-- ; return HIGH ; }

    unsigned int s = t % 60;
    unsigned int m = (t / 60) % 60;

    // pulse once per minute from 00 to 49
    if ( m > 49 ) return LOW ;

    if ( s == 0 ) return HIGH ;
    return LOW ;
}

//_____________________________________
// Implement the D-signal protocol.
// Returns HIGH or LOW depending on what the D-signal output should
// based on the current time.
//
// The 'D' signal is raised once per minute at zero-seconds for each
// minute.
int checkD(unsigned t) {
    unsigned int s = t % 60;

    // pulse once per minute

    if ( s == 0 ) return HIGH ;
    return LOW ;
}

// Flicker the LED if something interesting has happened
static int somethingHappened = 0;
static bool led = true;
void showActivity(int count) {
  if (!somethingHappened)
    somethingHappened = count*2 + 1;
}

static void ledService() {
  static int subTimer = 0;       ///< State counter per 100ms
  if (!subTimer) subTimer = getTick();

  int ledTime = (somethingHappened == 1) ? 10 : 1;
  if (elapsed(subTimer) < ledTime ) return ;
  subTimer += ledTime;

  if (somethingHappened) {
    if (--somethingHappened)
      led = !led;
  }

  digitalWrite(BUILTIN_LED, led ? LOW : HIGH);
}

void toggleLed() {
  led = !led;
}

// The time difference when we're 30 minutes fast
#define FAST_WAIT_THRESHOLD     (MAX_TIME - 30*60)

bool haveWallTime = false;
//_____________________________________
// Advances second and minute counters.
void markTime()
{
        static unsigned prev_t = 0;
        auto now = getRealTime();
        auto display = getWallTime() + 60;
        auto delta = (MAX_TIME + now - display) % MAX_TIME;

        a = b = d = LOW;

        if (prev_t == now) return;
        prev_t = now;

        if (!haveWallTime || !TimeService::hasBeenSynced()) {
                // If we don't know the clock position, we can't catch up
                delta = 0;
        }

        if (run_switch()) {
                // p(":RUN:");
                a = b = d = HIGH;
                resetWallTime();
                haveWallTime = true;
        }
        else if (delta > FAST_WAIT_THRESHOLD) {
                // Clock is fast, but it's less than 30 minutes fast.  Let's just wait for time to catch up
                // p(":FAST %ld:", MAX_TIME-delta, MAX_TIME-FAST_WAIT_THRESHOLD);

        } else if (delta > 60) {
                // Clock is 2+ minutes slow. Run until we catch up.
                // p(":SLOW %ld:", delta);
                a = b = d = HIGH;
                incMinutes();
        } else {
                // p(":ONTIME %ld:", delta);
                a = checkA(now) ;
                b = checkB(now) ;
                d = checkD(now) ;
                if ((now % 60) == 0) incMinutes();
        }

        // Once we know and saved the real time, assume we're in sync
        if (!haveWallTime && TimeService::hasBeenSynced()) {
                haveWallTime = saveTime();
                if (haveWallTime) resetWallTime();
        }

}

void clockSetup() {
        auto t = readTime();
        if (t>=0) {
                haveWallTime = true;
                setWallTime(t);
                t /= 60;
                p("Clock face: %02d:%02d\n", t/60, t%60);
        }
}

//_____________________________________
// the service routine runs over and over again forever:
void service() {
  static int subTimer = 0;       ///< State counter per 100ms

  consoleService() ;
  NtpService() ;
  ledService();

  switch (state) {
  default:
  case reset:
    subTimer = getTick() ;
  case rise:
    markTime();

    if (a||b||d) {
        sendSignal( a , b , d ) ;        // Send output pulses (if any)
        toggleLed();
        subTimer = getTick();
        state = riseWait;
    }
    showTime() ;                 // Report time and signals to serial port

    break ;

  case riseWait:
    if ( elapsed(subTimer) < riseTime ) break ;
    subTimer = getTick();
    state = fall;
    break;

  case fall:
    sendSignal( LOW , LOW , LOW ) ;     // End output pulses
    subTimer = getTick();
    toggleLed();
    showSignalDrop() ;

    state = fallWait;
    break;

  case fallWait:
    if ( elapsed(subTimer) < fallTime ) break ;
    subTimer += fallTime ;

    // Save new clock time, if it has changed
    saveTime();

    state = rise;
    break;
  }
}
