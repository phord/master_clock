// Stand-in for Arduino header on a PC
//

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#ifndef LOW
	// On the Arduino these are already defined.
	// On the PC they are not, so we define them here.
	#define LOW	0
	#define HIGH	1
#endif

void noInterrupts() ;
void interrupts() ;
