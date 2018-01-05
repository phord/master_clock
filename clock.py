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
import math

class Signal:
    A = LED(17)
    B = LED(27)
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

class Time:
    '''
    Keep track of needed adjustments to time, like lost pulses because of power failure
    and daylight savings time changes.
    '''
    H = 0
    M = 0
    S = 0

    def __init__(self, h=-1, m=0, s=0):
        if h < 0:
            self.reset()
        else:
            (self.H,self.M,self.S) = (h,m,s)
            self.M += self.S // 60
            self.H += self.M // 60
            self.S %= 60
            self.M %= 60
            self.H %= 24


    def __add__(self, other):
        return Time( self.H + other.H, self.M + other.M, self.S + other.S)

    def __lt__(self, other):
        return (self.H < other.H or
            (self.H == other.H and self.M < other.M) or
            (self.H == other.H and self.M == other.M and self.S < other.S))

    def __eq__(self, other):
        return self.H == other.H and self.M == other.M and self.S == other.S

    def reset(self):
        t = localtime()
        self.H = t.tm_hour
        self.M = t.tm_min
        self.S = t.tm_sec


class Simplex:
    def secondsUntilNextPulse(self, t):
        ''' Count expected seconds before next pulse will be sent.
        '''
        return min(secondsUntilNextPulseA(t), secondsUntilNextPulseB(t))

    def secondsUntilNextPulseA(self, t):
        '''
            Count seconds before next A pulse will be sent.
        '''
        if t.M != 59 or t.S > 50 or t.S == 0:
            return (60 - t.S) % 60

        if t.S >= 10:
            return t.S & 1

        # t.M == 59 and t.S > 0 and t.S < 10:
        return 10-t.S

    def secondsUntilNextPulseB(self, t):
        '''
            Count seconds before next B pulse will be sent.
        '''
        # pulse once per minute
        if t.M > 49 or (t.M == 49 and t.S > 0):
            return 60 - t.S + (59-t.M) * 60

        return (60 - t.S) % 60

    def checkA(self, t):
        '''
            Implement the A-signal protocol.
            Returns HIGH or LOW depending on what the A-signal output should
            based on the current time.

            The 'A' signal is raised once per minute at zero-seconds, and on
            every odd second between 10 and 50 during the 59th minute.
        '''

        # pulse once per minute
        if t.S == 0:
            return True

        if t.M == 59:
            if t.S >= 10 and t.S <= 50:
                if (t.S & 1) == 0:
                    return True

        return False

    def checkB(self, t):
        '''
            Implement the B-signal protocol.
            Returns HIGH or LOW depending on what the B-signal output should
            based on the current time.

            The 'B' signal is raised once per minute at zero-seconds for each
            minute except for minutes 50 to 59.
        '''
        # pulse once per minute
        if t.M > 49:
            return False

        if t.S == 0:
            return True

        return False


class Console:
    # Print the current time and A/B signal levels on the console
    def showTime(self, tm, a, b):
        s = tm.S
        #-- Newline + whole time every minute
        if s == 0:
            print("\n{:02d}:{:02d}:{:02d} ".format(tm.H,tm.M, s), end='', flush=True)
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

def test():
    protocol = Simplex()
    pA = 1
    pB = 1
    for m in range(60):
        for s in range(60):
            tm = Time(0,m,s)

            tA = protocol.secondsUntilNextPulseA(tm)
            tB = protocol.secondsUntilNextPulseB(tm)

            fA = protocol.checkA(tm)
            fB = protocol.checkB(tm)

            print("{:02}:{:02}  A:{:02}/{}  B:{:02}/{}".format(tm.M,tm.S, tA, fA, tB, fB))

            if pA>0 and pA - tA != 1:
                raise "Protocol error: A"
            if pB>0 and pB - tB != 1:
                raise "Protocol error: B"

            if fA is not (tA == 0):
                raise "Protocol error: A signal"
            if fB is not (tB == 0):
                raise "Protocol error: B signal"

            pA = tA
            pB = tB
def main():
    # test()
    run()

def run():
    signal = Signal()
    protocol = Simplex()
    con = Console()
    signal.send(False, False)

    # TODO: Read last known displayed time from log file
    display_time = Time()
    display_time.S = 0

    if not signal.checkPower():
        con.showPowerLoss(display_time, display_time)

    while True:
        t = Time()
        if t == display_time:
            sleep(0.1)
            continue

        a = False
        b = False

        # Advance display timer only if we have power; otherwise we can't advance it, so don't pretend
        if signal.checkPower():
            display_time += Time(0,0,1)
            a = protocol.checkA(display_time)
            b = protocol.checkB(display_time)

        # RUN switch overrides protocol; if held pulse both lines every second
        if signal.checkRun():
            a = True
            b = True

        # Report time and signals to console
        if signal.checkPower():
            con.showTime(display_time, a, b)
        else:
            con.showPowerLoss(display_time, t)
            sleep(1)

        if a or b:
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
