/*
   MasterClock - Drives a slave clock using the Internation Business Machine Time Protocols,
  Service Instructions No. 222, April 1, 1938,Form 231-8920
*/
#include "Arduino.h"
#include "clock_generic.h"
#ifndef LOW
	#define LOW	0
	#define HIGH	1
#endif

static int m, s ;
int a = LOW , b = LOW ;
int aForce = 0 ;
int bForce = 0 ;

void syncTime() ;

int tick = 0;
void ticker() {
  ++tick;
}

void setTick(int adjust) {
  noInterrupts() ;
  tick += adjust ;
  interrupts();
}

int getTick() {
  noInterrupts() ;
  int x = tick ;
  interrupts();
  return x;
}

#include <stdarg.h>
void p(char *fmt, ... ){
        char tmp[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(tmp, 128, fmt, args);
        va_end (args);
        sendString(tmp);
}

void showTime() {
  p("\n%c%c %02u:%02u   " , (a?'A':' ') , (b?'B':' '), m, s );
}

void markTime() {
  ++s ;
  if ( s >59 ) {
    s = 0 ;
    ++m ;
  }
  if ( m > 59 ) m = 0;
}

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

int checkB() {
    // Manual run based on serial input
    if ( bForce ) { bForce-- ; return HIGH ; }

    // pulse once per minute from 00 to 49
    if ( m > 49 ) return LOW ;

    if ( s == 0 ) return HIGH ;
    return LOW ;
}

static bool timeEntryMode = false ;

bool timeEntry( char ch ) {
static char buf[10] ;
static int ibuf = 0 ;

  buf[ibuf]   = 0 ;

  // Read the time input from the serial port
  if ( isdigit(ch) || ch == ':' )
  {

    if ( ibuf < sizeof(buf)-1 )
      buf[ibuf++] = ch ;

    buf[ibuf] = 0 ;
    return false ;
  }

  switch ( ch ) {
    case 27 : case 3: ibuf = 0 ; timeEntryMode = false ; return false ;
    case 8  : if ( ibuf > 0 ) buf[--ibuf] = '\0'; return false ;
  }

  timeEntryMode = false ;
  if ( ibuf == 0 ) return false ;

  p ("    time=%s ", buf);

  // interpret the time we have accumulated from the user
  int m0 = -1 ;
  int s0 = -1 ;
  char *pp = buf ;

  // Find any colon the user entered
  for ( ; *pp && *pp != ':' ; pp++ ) ;

  // :00    pp == buf
  // 0000   *(pp+1) == 0 && ibuf > 2
  // 00:    *(pp+1) == 0
  // 00:00  *pp=':'

  if ( *pp==':' ) {
    // nn:...  User entered minutes value
    if ( pp > buf ) m0 = atoi(buf) ;

    // :nn  User entered seconds value after colon
    if ( pp[1]>0 ) s0 = atoi(pp+1);
  }
  else {
    s0 = atoi(buf) ;
    if ( ibuf > 2 ) {
      m0 = s0 / 100 ;
      s0 %= 100 ;
    }
  }

  ibuf = 0 ;

  if ( m0 > 59 || s0 > 59 )
  {
    p("  ** Bad time input ** ");
    return false ;
  }

  if ( s0 >= 0 ) syncTime() ;
  if ( m0 < 0 ) m0 = m ;
  if ( s0 < 0 ) s0 = s ;

  m = m0 ; s = s0 ;

  return true ;
}

bool controlMode( char ch ) {
    switch ( ch ) {

    // PLUS advances the digital clock minute
    case '+': case '=': if (++m > 59) m=0 ; return true ;

    // MINUS decrements the digital clock minute
    case '_': case '-': if (--m < 0) m=59 ; return true ;

    // 'Z' resets the digital clock seconds
    case 'z': case 'Z': s = 0 ; syncTime() ; return true ;

    // A, B and C manually force the A/B output pulses but do not affect the internal clock
    // A and B add to the force count for the A and B signals.  C adds to both signals.
    case 'A': case 'a': ++aForce ;            break ;
    case 'B': case 'b':            ++bForce ; break ;
    case 'C': case 'c': ++aForce ; ++bForce ; break ;
  }
  return false ;
}

void serialControl() {
  char ch ;
  bool timeChange = false ;

  ch = readKey();
  if ( ch < 1 ) return ;

  if ( isdigit(ch) || ch == ':' ) timeEntryMode = true ;

  if ( timeEntryMode )
  {
    if ( timeEntry( ch ) )
    {
      timeEntryMode = false ;
      timeChange = true ;
    }
  }

  timeChange |= controlMode(ch) ;

  if ( timeChange ) { p(" -> %02d:%02d ", m, s);  }
}

typedef enum State { riseWait , rise, fallWait , fall } State ;
State state = rise ;

// Signal output duration, in 10ms ticks
enum {
	riseTime = 6 ,     // 600ms rise time
	fallTime = 4 ,     // 400ms fall time
} ;

void syncTime() {
  setTick( -getTick() ) ;
  state = rise ;
}

// the service routine runs over and over again forever:
void service() {
  serialControl() ;

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
    p("%s", (a||b)?"off ":"");

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
