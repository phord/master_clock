#!/usr/bin/python3

#
## master clock driver
#
# Master Clock - Drives an IBM Impulse Secondary clock movement
# using the International Business Machine Time Protocols,
# Service Instructions No. 230, April 1, 1938,Form 231-8884-0
# By Phil Hord,  This code is in the public domain January 2, 2018
#

from gpiozero import LED, Button
from time import sleep, localtime
from SimplexProtocol import Time, Simplex
import math
import os

# If the clock is fast, but less than this many minutes fast, wait to catch up instead
# advancing all the way around the clock again.
FAST_WAIT_THRESHOLD = 30

class Signal:
    A = LED(27)
    B = LED(17)
    RUN = Button(22)
    POWER = Button(5, pull_up=False)

    # pulse width in milliseconds
    pulse_width = (600, 400)

    def checkRun(self):
        return self.RUN.is_pressed

    def checkPower(self):
        return self.POWER.is_pressed

    def send(self, a, b):
        if a: self.A.on()
        else: self.A.off()

        if b: self.B.on()
        else: self.B.off()

class Console:
    # Print the current time and A/B signal levels on the console
    def showTime(self, tm, a, b, msg=""):
        s = tm.S

        if msg:
            print("  {}  ".format(msg), end='')

        #-- Newline + whole time every minute
        if s == 0:
            print("\n{} ".format(tm), end='', flush=True)
        elif (s % 10 ) == 0:
            print("{0:02d}".format(s), end='', flush=True)
        else:
            print("-", end='', flush=True)

        #-- Show the A/B pulse status
        if a: print("A", end='', flush=True)
        if b: print("B", end='', flush=True)

    def showPowerLoss(self, since, now):
        if now.S == 0:
            print("\nPower outage since {:02d}:{:02d}:{:02d} ".format(since.H,since.M, since.S), end='', flush=True)
        self.showTime(now, False, False)

    # Report the A or B signal has dropped
    def showSignalDrop(self, a, b):
        if a or b:
            print("*", end='', flush=True)

def main():
    signal = Signal()
    protocol = Simplex()
    con = Console()

    # Initialize outputs to OFF
    signal.send(False, False)

    displayTimeCacheFile = "displayed-time.dat"
    displayTimeUserOverride = "user-time.dat"

    if not signal.checkPower():
        con.showPowerLoss(display_time, display_time)

    print("Master clock start.  ", end='')
    try:
        # Read last known displayed time from log file
        display_time = Time.load(displayTimeCacheFile)
        print("Display time: {} (trusted={})".format(display_time, display_time.trusted()))

        # Use the clock display time as our RTC if we're not synched yet
        Time.set_base_time(display_time)
    except:
        display_time = Time()
        display_time.trust(False)
        print("Failed to parse display time in {}".format(displayTimeCacheFile))
        pass

    while True:
        # Check if user-time file exists.
        # If so, use the time from there and delete the file
        try:
            userTime = Time.load(displayTimeUserOverride)
            os.unlink(displayTimeUserOverride)
            display_time = userTime
            print("\nUser set display time: {}\n".format(display_time))
            display_time.save(displayTimeCacheFile)
        except:
            pass

        t = Time()
        if t == display_time:
            sleep(0.1)
            continue


        synced = Time.ntp_syncronized()

        # Assume display time matches synced time if we don't know otherwise
        if synced and not display_time.trusted():
            display_time.reset()

        if not signal.checkPower():
            # Power loss; mark time and wait for power to come back on
            con.showPowerLoss(display_time, t)
            sleep(30)
            continue

        if signal.checkRun():
            # RUN switch overrides protocol, when powered; if held pulse both lines every second
            msg = "RUN switch"
            a = True
            b = True

            # If user is running the show, forget any lag time we thought we would catch up
            display_time.reset()

        elif (display_time - t) > 0 and (display_time - t) / 60 <= FAST_WAIT_THRESHOLD:
            # Clock is fast, but it's less than 30 minutes fast.  Let's just wait for time to catch up
            msg = "Fast: WAIT {}".format(t, display_time)
            sleep(min(60-t.S,5))
            a = False
            b = False

        elif t - display_time > 100:
            # Clock is way behind.  Skip over the Simplex corrections to run as fast as we can
            msg = "Slow: CATCH-UP {} {}".format(t, (t - display_time)//60)
            a = True
            b = True
            display_time += Time(0,1,0)

        elif not synced and (t.M&1) == 0:
            # Double-pulse every minute to alert user we don't have ntp
            msg = "No NTP Sync"
            sleep(min(60-t.S,5))
            a = False
            b = False

        else:
            # Advance display time by one second
            msg = ""
            display_time += Time(0,0,1)
            a = protocol.checkA(display_time)
            b = protocol.checkB(display_time)


        con.showTime(display_time, a, b, msg)

        if a or b:
            if display_time.trusted():
                # Record clock time in case of power loss
                display_time.save(displayTimeCacheFile)

            # Send output pulses, if any
            signal.send(a, b)

            # Hold pulse high for pulse_width time
            sleep(0.001 * signal.pulse_width[0])

            # Drop the signal
            signal.send(False, False)

            con.showSignalDrop(a, b)

            # Hold pulse low for low-side of pulse-width at least
            sleep(0.001 * signal.pulse_width[1])

if __name__ == "__main__":
    main()
