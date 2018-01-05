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

if __name__ == "__main__":
    test()
