all:
	g++ -I . -I pc pc/pc.c pc/kbhit.c clock_generic.cpp console.cpp

mingw:
	i586-mingw32msvc-g++ -I . -I pc pc/pc.c pc/kbhit.c clock_generic.cpp console.cpp
