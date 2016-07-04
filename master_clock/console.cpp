/*
   console.cpp

   Implements the console user interface to the master clock.

    Master Clock - Drives an IBM Impulse Secondary clock movement
    using the International Business Machine Time Protocols,
    Service Instructions No. 230, April 1, 1938,Form 231-8884-0
    By Phil Hord,  This code is in the public domain Sept 9, 2013
*/

//_____________________________________________________________________
//                                                             INCLUDES
#include "Arduino.h"
#include <stdarg.h>
#include "clock_generic.h"
#include "console.h"

//_____________________________________________________________________
// Print formatted text to the console.
void p(const char *fmt, ... ){
        char tmp[128]; // resulting string limited to 128 chars
        memset(tmp,0,sizeof(tmp));
        va_list args;
        va_start (args, fmt );
        vsnprintf(tmp, 128, fmt, args);
        va_end (args);
        sendString(tmp);
}

// Print the current time and A/B signal levels on the console
void showTime() {
  unsigned int s = getSeconds();

  //-- Newline + whole time every minute
  if ( s == 0 )
    p("%02u:%02u %c%c\n", getMinutes(), s, getA()?'A':' ', getB()?'B':' ');
//  else if ( (s % 10 ) == 0)
//    p("%02u", s );
//  else
//    p("-");

//  //-- Show the A/B pulse status
//  if (getA()) p("A");
//  if (getB()) p("B");
//  if ( s == 0 || getA() || getB()) p("\n");
}

// Report the A or B signal has dropped
void showSignalDrop() {
//    if (getA()||getB()) p("*\n");
}

static bool timeEntryMode = false ;

//_____________________________________
// Parse time values input by the user
bool timeEntry( char ch ) {
  static char buf[10] ;          ///< Collect the entered time string
  static unsigned int ibuf = 0 ; ///< Count number of entered characters

  buf[ibuf]   = 0 ;              ///< Zero-terminate the string so far

  // Read the time input (numbers and ':' only)
  if ( isdigit(ch) || ch == ':' )
  {

    if ( ibuf < sizeof(buf)-1 )
      buf[ibuf++] = ch ;

    buf[ibuf] = 0 ;
    return false ;
  }

  // Process 'special' keys
  switch ( ch ) {
    case 27 :     // ESC
    case 3 :      // Ctrl-C
	    ibuf = 0 ;                   // Cancel the user input
	    timeEntryMode = false ;      // Exit time-entry-mode
	    return false ;

    case 8  :     // Backspace
	    if ( ibuf > 0 ) buf[--ibuf] = '\0';    // Erase the last-entered byte
	    return false ;
  }

  // If we get this far, the user typed something which is not a valid time character.
  // End the time entry mode and set the time values if they are valid.

  timeEntryMode = false ;
  if ( ibuf == 0 ) return false ;

  // Show the user the time string he entered
  p ("    time=%s ", buf);

  // interpret the time we have accumulated from the user
  int m0 = -1 ;
  int s0 = -1 ;
  char *pp = buf ;

  // Acceptable time formats:
  // mm:ss  Set the minutes and the seconds
  // mmss   Set the minutes and the seconds
  // :ss    Set the seconds, but leave the minutes alone
  // ss     Set the seconds, but leave the minutes alone
  // mm:    Set the minutes, but leave the seconds alone

  // Find any colon the user entered
  for ( ; *pp && *pp != ':' ; pp++ ) ;

  // Here's how we recognize the different formats
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
    // ss or mmss
    s0 = atoi(buf) ;
    if ( ibuf > 2 ) {
      // mmss
      m0 = s0 / 100 ;
      s0 %= 100 ;
    }
  }

  ibuf = 0 ;

  // Complain about bad input
  if ( m0 > 59 || s0 > 59 )
  {
    p("  ** Bad time input ** ");
    return false ;
  }

  // Set the clock time to the values the user entered
  if ( m0 >= 0 ) setMinutes(m0);
  if ( s0 >= 0 ) setSeconds(s0);

  // If we changed the seconds value, reset the timer-tick to 00 milliseconds
  if ( s0 >= 0 ) syncTime() ;

  return true ;
}

void triggerNtp();
//_____________________________________
// Read user input and act on it
bool controlMode( char ch ) {
    switch ( ch ) {

    // 'N' triggers the NTP protocol manually
    case 'N': case 'n': triggerNtp() ; return true ;

    // Left-arrow (Less-Than) slows down the clock for simulation runs
    case '<': case ',': slowDown() ; return true ;

    // Right-arrow (Greater-Than) speeds up the clock for simulation runs
    case '>': case '.': speedUp() ; return true ;

    // PLUS advances the digital clock minute
    case '+': case '=': incMinutes() ; return true ;

    // MINUS decrements the digital clock minute
    case '_': case '-': decMinutes() ; return true ;

    // 'Z' resets the digital clock seconds
    case 'z': case 'Z': setSeconds(0) ; syncTime() ; return true ;

    // A, B and C manually force the A/B output pulses but do not affect the internal clock
    // A and B add to the force count for the A and B signals.  C adds to both signals.
    case 'A': case 'a': sendPulseA() ;               break ;
    case 'B': case 'b':                sendPulseB() ; break ;
    case 'C': case 'c': sendPulseA() ; sendPulseB() ; break ;
  }
  return false ;
}

void consoleService() {
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

  if ( timeChange ) { p(" -> %02d:%02d ", getMinutes(), getSeconds() );  }
}
