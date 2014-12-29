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
