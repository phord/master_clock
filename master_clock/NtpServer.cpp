/**
 * NTP service state machine.
 *
 * Syncrhonizes time with NTP server once per hour.
 * Allows user to initiate one-off synchronization
 * as needed.
 */

//_____________________________________________________________________
//                                                             INCLUDES
#include "Arduino.h"
#include "console.h"
#include "clock_generic.h"
#include "Timer.h"
#include "ntp.h"
#include "Udp.h"


//_____________________________________________________________________
//                                                                LOCAL

typedef enum Ntp_State {
    ntp_idle ,          ///< NTP is waiting to be triggered
    ntp_cron ,          ///< Auto-trigger NTP request
    ntp_request ,       ///< Sending NTP request
    ntp_response ,      ///< Waiting for NTP response
    ntp_completed ,     ///< NTP completed; waiting for reset
    ntp_oneshot ,       ///< Try a short NTP request (manual intervention)
} Ntp_State ;

static Ntp_State ntpState = ntp_idle ;

//_____________________________________________________________________
//                                                                 CODE

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

  long delta = (now % 3600L ) - (getMinutes()*60+getSeconds()) ;
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

void triggerNtp() {
	ntpState = ntp_oneshot ;
}

void NtpSetup() {
    ntpSetup() ;
    triggerNtp() ;
}

void NtpService() {
    static int ntpResponseTimeout = 0 ;
    static int ntpRequestGuardTime = 0 ;
    static int retries ;

//	p("\r%d:  %d  %d  %d    ", ntpState , realTick, ntpResponseTimeout , retries ) ;
    switch ( ntpState ) {
    case ntp_idle :
        if ( getMinutes() == 58 ) ntpState = ntp_cron ;
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
