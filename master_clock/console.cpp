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
  auto t = getRealTime();
  unsigned int s = t % 60;
  unsigned int m = (t / 60) % 60;
  unsigned int h = (t / 60 / 60) % 12;

  auto wt = getWallTime();
  unsigned int wm = (wt / 60) % 60;
  unsigned int wh = (wt / 60 / 60) % 12;

  //-- Newline + whole time every minute
  if ( s == 0 )
    p("\n%02u:%02u:%02u %02u:%02u ", h, m, s, wh, wm);
  else if ( (s % 10 ) == 0)
    p("%02u", s );
  else
    p("-");

  //-- Show the A/B pulse status
  if (getA()) p("A");
  if (getB()) p("B");
}

// Report the A or B signal has dropped
void showSignalDrop() {
  p("%s", (getA()||getB())?"*":"");
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


  return true ;
}

//_____________________________________
// Read user input and act on it
bool controlMode( char ch ) {
    switch ( ch ) {

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

//   timeChange |= controlMode(ch) ;

//   if ( timeChange ) { p(" -> %02d:%02d ", getMinutes(), getSeconds() );  }
}
