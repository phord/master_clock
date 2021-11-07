master_clock
============

Master clock protocol driver for IBM slave clocks using Simplex Protocol from an ESP8266/Arduino or Raspberry pi.

The original build of this master clock controller is described in the
January 2015 issue of Nuts and Volts:

    http://nutsvolts.texterity.com/nutsvolts/201501/\?folio\=32\&pg\=32\#pg32

Note: You may need a subscription to use the link.

An updated PDF is included in this repo in `Arduino based Master ClockV2.pdf`. It
details the protocol, original circuit designs, and some code usage.

The project has grown since then to support ESP wifi controllers and Raspberry Pi embedded Linux computers.

Accordingly, there are two codebases here for different targets.
## ESP8266, Arduino mode (C++)
**Directory: master_clock/**

The current code builds and runs on an ESP8266 module from Espressif. The code is configured for the NodeMCU 1.0, but it
should adapt easily to other boards.  In the past I ran it on the Digistump Oak boards.

This version relies on the ESP libraries to provide time sync, which they do automatically.

You can build this with the Arduino IDE or with the CLI like this:

    arduino-cli -v compile -b esp8266:esp8266:nodemcuv2 --build-cache-path ../build master_clock

## Raspberry Pi (Python)
**Directory: raspi/**

The entire project is reimplemented here with all the original features and a few new ones. It runs on a Raspberry Pi
under any Linux OS. There is a rudimentary install script to help set it up and to enable it as a service so it is
auto-started on boot.

**clock.py** is the actual program. It also imports **SimplexProtocol.py**.  You shouldn't need to run this manually.

**setclock.py** is used to tell the clock service what time is currently displayed on the clock face.

    ./setclock.py 9 50

**clock-status.py** is mostly for debugging. It reads the stored-time files and reports on the current system time.



## Arduino (No longer supported)

The original code ran on a regular Arduino using a WiFi or Network shield. It had some dodgy NTP-sync code to sync the
time with the NIST time servers. It had a few limitations, such as hard-coding the IP address for the time server
instead of finding it with DNS in the pool.  This code is not directly supported any more, but you can find the
original Arduino code in the git history if you want it.

## PC simulator (Posix C++, no longer supported)
**Directory: pc/**

The Arduino code also compiles for a PC platform using some stubs to mock the Arduino interfaces. This was useful for
testing as a simulator early on, but it hasn't been kept up-to-date lately.

Other hardware interfaces could be added easily enough. The Arduino is pretty specific about its code layout, but other interfaces are not so persnickity.
