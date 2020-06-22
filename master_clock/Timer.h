// Timer.h
//
// Time wrappers
//
//    Master Clock - Drives an IBM Impulse Secondary clock movement
//    using the International Business Machine Time Protocols,
//    Service Instructions No. 230, April 1, 1938,Form 231-8884-0
//    By Phil Hord,  This code is in the public domain Sept 9, 2013
//

//________________________________________________________________
// Tick counter
//
// The ticker function is called at precise times by the main
// program somehow.  This is how we keep track of time.  On
// the Arduino this function is called by a hardware interrupt.
// On the PC it is called by a simple clock watcher.
//
void ticker() ;

int getTick() ;
int elapsed( int timer ) ;
int getFuture( int nSeconds ) ;
bool expired( int timer ) ;
