master_clock
============

Master clock protocol driver for IBM slave clocks using Simplex Protocol from an ESP8266/Arduino or Raspberry pi.

The included article, [Arduino based Master ClockV2.pdf](Arduino based Master ClockV2.pdf), was published in Nuts &
Volts Magazine about this project. It details the protocol, circuit designs, and code usage.

There are three codebases here for three different targets.
## ESP8266, Arduino mode (C++)
**Directory: master_clock/**

The current code builds and runs on an ESP8266 module from Espressif. The code is configured for the NodeMCU 1.0, but it
should adapt easily to other boards.  In the past I ran it on the Digistump Oak boards.

This version relies on the ESP libraries to provide time sync, which they do automatically.

## Raspberry Pi (Python)
**Directory: raspi/**

The entire project is reimplemented here with all the original features and a few new ones. It runs on a Raspberry Pi
under any Linux OS. There is an install script to help set it up and to enable it as a service so it is auto-started on
boot.

## Arduino

The original code ran on a regular Arduino using a WiFi or Network shield. It had some dodgy NTP-sync code to sync the
time with the NIST time servers. It had a few limitations, such as hard-coding the IP address for the time server
instead of finding it with DNS in the pool.

## PC (Posix C++) for testing
**Directory: pc/**

The Arduino code also compiles for a PC platform using some stubs to mock the Arduino interfaces. This was useful for
testing as a simulator early on, but it hasn't been kept up-to-date lately.

Other hardware interfaces could be added easily enough. The Arduino is pretty specific about its code layout, but other interfaces are not so persnickity.
