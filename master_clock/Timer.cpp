/*
    Master Clock - Drives an IBM Impulse Secondary clock movement
    using the International Business Machine Time Protocols,
    Service Instructions No. 230, April 1, 1938,Form 231-8884-0
    By Phil Hord,  This code is in the public domain Sept 9, 2013
*/

//_____________________________________________________________________
//                                                             INCLUDES
#include "Arduino.h"
#include "Timer.h"

//_____________________________________________________________________
//                                                           LOCAL VARS

static int realTick = 0;       ///< The number of ticks since we started

//_____________________________________________________________________
// Increment the tick counter.  This is called once every 100ms by the
// hardware interrupt or main executive function.
void ticker() {
  ++realTick;
}

//_____________________________________
// Read the current 10ms tick variable.
int getTick() {
  return millis()/100;
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
