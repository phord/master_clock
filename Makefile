
.INTERMEDIATE:  clock_generic.cpp console.cpp ntp.cpp

all: avr clock
PC_SOURCES=pc.c kbhit.c udp.cpp
COMMON_SOURCES=clock_generic.cpp Timer.cpp console.cpp ntp.cpp NtpServer.cpp ConfigData.cpp Config.cpp

SOURCES=$(addprefix pc/,$(PC_SOURCES)) $(addprefix master_clock/,$(COMMON_SOURCES))

clock: $(SOURCES) Makefile
	g++ -I master_clock -I pc $(SOURCES) -o clock

avr upload monitor clean::
	make -sC master_clock $(MAKECMDGOALS)

clean::
	rm clock
