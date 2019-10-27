#!/usr/bin/python3

#
## Simplex Master Clock protocol
#
# SimplexProtocol -Implements International Business Machine Time Protocols,
# Service Instructions No. 230, April 1, 1938,Form 231-8884-0
# By Phil Hord,  This code is in the public domain January 2, 2018
#

import math
import os
import pickle
from time import sleep, monotonic, localtime
import ntplib

class Time:
    '''
    Keep track of needed adjustments to time, like lost pulses because of power failure
    and daylight savings time changes.
    '''

    # Remember whether we are synced and when we last checked
    mem_synchronized = False
    mem_synch_time = 0

    base_time = 0

    def __init__(self, h=None, m=0, s=0):
        if h is None:
            self.reset()
        else:
            (self.H,self.M,self.S) = (h,m,s)
        self.normalize()

    def normalize(self):
        self.M += self.S // 60
        self.H += self.M // 60
        self.S %= 60
        self.M %= 60
        self.H %= 12
        if self.H == 0:
            self.H = 12

    def __repr__(self):
        return "<Time: H:{} M:{} S:{}>".format(self.H, self.M, self.S)

    def __str__(self):
        return "{:02d}:{:02d}:{:02d}".format(self.H, self.M, self.S)

    def __add__(self, other):
        return Time( self.H + other.H, self.M + other.M, self.S + other.S)

    def __sub__(self, other):
        T = Time( self.H - other.H, self.M - other.M, self.S - other.S)
        return T.S + T.M * 60 + (T.H % 12) * 3600

    def __lt__(self, other):
        return (self.H < other.H or
            (self.H == other.H and self.M < other.M) or
            (self.H == other.H and self.M == other.M and self.S < other.S))

    def __eq__(self, other):
        return self.H == other.H and self.M == other.M and self.S == other.S

    def reset(self, force_monotonic=False):
        if not force_monotonic and Time.ntp_syncronized():
            t = localtime()
            self.H = t.tm_hour
            self.M = t.tm_min
            self.S = t.tm_sec
        else:
            self.H = 0
            self.M = 0
            self.S = int(monotonic() - Time.base_time)

        self.normalize()

    def save(self, fname):
        fo = open( fname, "wb" )
        pickle.dump(self, fo)
        os.fsync(fo)
        fo.close()

    def load(fname):
        t = pickle.load(open( fname, "rb" ) )
        t.normalize()
        return t

    @staticmethod
    def set_base_time(t):
        # we will use this as our base time until we're ntp synchronized
        Time.base_time = monotonic() - (t - Time(0))

    @staticmethod
    def synch_time_elapsed(delta):
        return abs(Time.mem_synch_time - monotonic()) > delta

    @staticmethod
    def force_ntp_syncronized(synced=True):
        Time.mem_synchronized = synced
        Time.mem_synch_time = monotonic()

    @staticmethod
    def ntp_syncronized():
        # Once we're synced, don't check again for 24 hours
        if Time.mem_synchronized and not Time.synch_time_elapsed(24 * 3600):
            return Time.mem_synchronized

        # If we're out of sync, check again every 5 minutes
        if not Time.mem_synchronized and not Time.synch_time_elapsed(5 * 60):
            return Time.mem_synchronized

        ofs = Time.ntp_offset()
        # Assume we're in sync if the ntp time is with 10 seconds of our clock
        Time.mem_synchronized = ofs is not None and abs(ofs) < 10
        Time.mem_synch_time = monotonic()
        if Time.mem_synchronized:
            Time.set_base_time(Time())
        return Time.mem_synchronized

    @staticmethod
    def ntp_offset(timeout=15):
        try:
            c = ntplib.NTPClient()
            response = c.request('pool.ntp.org', version=3, timeout=timeout)
            return response.offset
        except:
            return None

    def self_test(self):
        # Avoid hitting the network for tests
        Time.force_ntp_syncronized(False)

        a = Time(1,2,3)
        b = Time(5,6,7)
        c = Time(11,57,58)

        if a+b == c:
            raise "Time math makes no sense"

        if a-b < 0 or b-a < 0:
            raise "12hr time deltas should always be positive"

        if a + Time(0,0,b-a) != b:
            raise "Addition is not the opposite of subtraction"

        if a + Time(0,0,c-a) != c:
            raise "Addition is not the opposite of subtraction"

        if c + Time(0,0,a-c) != a:
            raise "Addition is not the opposite of subtraction"

        if not Time(11,59,60) == Time(12,0,0):
            print ("Time {} != {}".format(Time(11,59,60), Time(12,0,0)))
            raise "Time overflow not handled right"

        if not Time(-1,-10,-1) == Time(10,49,59):
            print ("Time {} != {}".format(Time(-1,-10,-1), Time(10,49,59)))
            raise "Time underflow not handled right"

        # TODO: add some subtraction tests

        if a-b != 3600*12 - (b-a):
            raise "Time subtraction failed"

        if b-a != 3661*4:
            raise "Time subtraction wrong answer"

        if a+b != Time(6,8,10):
            raise "Time addition failed"

        if b+c != Time(5,4,5):
            raise "Time addition didn't wrap correctly"

        t = Time(11,12,13)
        Time.set_base_time(t)
        t0 = Time()
        t0.reset()

        if int(t0 - t) != 0:
            print( "Monotonic clock base math fail {}".format(t0-t) )
            raise "Monotonic error"

        sleep(5)
        t1 = Time()
        t1.reset()
        if int(t1 - t0) != 5:
            print( "Monotonic clock is not normal {}".format(t1-t0) )
            raise "Monotonic error"

        print("%s self_test passed                 " % (__class__) )


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


    def self_test(self):
        pA = 1
        pB = 1
        for m in range(60):
            for s in range(60):
                tm = Time(0,m,s)

                tA = self.secondsUntilNextPulseA(tm)
                tB = self.secondsUntilNextPulseB(tm)

                fA = self.checkA(tm)
                fB = self.checkB(tm)

                print("{:02}:{:02}  A:{:02}/{}  B:{:02}/{}        ".format(tm.M,tm.S, tA, fA, tB, fB), end="\r")

                # check that "time until next pulse" is decreasing monotonically
                if pA>0 and pA - tA != 1:
                    raise "Protocol error: A"
                if pB>0 and pB - tB != 1:
                    raise "Protocol error: B"

                # check that pulse fires iff secondsUntilNextPulse is zero
                if fA is not (tA == 0):
                    raise "Protocol error: A signal"
                if fB is not (tB == 0):
                    raise "Protocol error: B signal"

                pA = tA
                pB = tB
        print("%s self_test passed                 " % (__class__) )

if __name__ == "__main__":
    protocol = Simplex()
    protocol.self_test()

    Time().self_test()
