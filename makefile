
.INTERMEDIATE:  clock_generic.cpp console.cpp ntp.cpp

%.cpp: %.ino
	cp $< $@

all: pc/pc.c pc/kbhit.c pc/udp.cpp clock_generic.cpp console.cpp ntp.cpp
	g++ -I . -I pc $^ -o clock
