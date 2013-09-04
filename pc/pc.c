/*
 * PC test harness
*/

#include <sys/time.h>
#include <stdio.h>
#include "kbhit.h"
#include "clock_generic.h"

extern int a , b ;

//__________________________________________________________________________
//								  INTERRUPTS
void noInterrupts() {}
void interrupts() {}

//__________________________________________________________________________
//								 SERIAL PORT
//_____________________________________
// Send a message to the user
void sendString(const char * str)
{
        printf("%s",str);
	fflush(NULL) ;
}

int quit = 0 ;
//_____________________________________
// Check for user input
char readKey()
{
	char ch = -1 ;
	if ( kbhit() ) {
		ch = readch() ;
		//printf("%d\n", ch );
		if ( ch == 3 || ch == 27 ) quit = 1 ;
	}
	return ch ;
}

//__________________________________________________________________________
//								 DIGITAL I/O
//_____________________________________
// Send the output signals to A and B
void sendSignal( int a, int b)
{
	// Nothing to do on the PC Simulator
}


//__________________________________________________________________________
//								  CLOCK TICK
//_____________________________________
// Return a microsecond counter
suseconds_t getMicroSeconds()
{
        struct timeval tv;
	struct timezone tz;

        gettimeofday(&tv, &tz);
	return tv.tv_sec * 1000000 + tv.tv_usec ;
}
//_____________________________________
// Increment the clock tick 10 times per second
static suseconds_t lastTime = 0 ;
int US_PER_TICK = 100000 ;
void timerService()
{
	if ( getMicroSeconds() - lastTime < US_PER_TICK ) return ;
	ticker() ;
	lastTime += US_PER_TICK ;
}

void speedUp() { US_PER_TICK /= 2 ; }
void slowDown() { US_PER_TICK *= 2 ; }

//_____________________________________
void timerInit()
{
	lastTime = getMicroSeconds();
}

//__________________________________________________________________________
//								        MAIN
//_____________________________________
int main ( int argc , char ** argv )
{
	init_keyboard();
	timerInit() ;
	clockSetup();

	while ( !quit )
	{
		// Call our timer service to ensure ticks get fed
		timerService() ;

		// Simulate the Arduino run-time
		service() ;
	}

	close_keyboard();
}
