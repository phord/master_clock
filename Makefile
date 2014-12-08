
.INTERMEDIATE:  clock_generic.cpp console.cpp ntp.cpp

all: avr pc

pc: pc/pc.c pc/kbhit.c pc/udp.cpp master_clock/clock_generic.cpp master_clock/console.cpp master_clock/ntp.cpp
	g++ -I master_clock -I pc $^ -o clock

avr upload monitor clean::
	make -sC master_clock $(MAKECMDGOALS)

clean::
	rm clock
