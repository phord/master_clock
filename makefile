
.INTERMEDIATE:  clock_generic.cpp console.cpp ntp.cpp

%.cpp: %.ino
	cp $< $@

all: avr pc

pc: pc/pc.c pc/kbhit.c pc/udp.cpp clock_generic.cpp console.cpp ntp.cpp
	g++ -I . -I pc $^ -o clock

avr upload monitor clean::
	make -sf makefile-avr $(MAKECMDGOALS)

clean::
	rm *.o clock
