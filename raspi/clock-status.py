#!/usr/bin/python3

from time import sleep, localtime
from SimplexProtocol import Time, Simplex
import math
import os

def main():
    protocol = Simplex()

    displayTimeCacheFile = "displayed-time.dat"
    displayTimeUserOverride = "user-time.dat"

    t = Time()
    print("System time: {} sync={}".format(t, Time.ntp_syncronized()))

    display_time = Time.load(displayTimeCacheFile)
    try:
        # Read last known displayed time from log file
        display_time = Time.load(displayTimeCacheFile)
        print("Display time: {}".format(display_time))
    except:
        print("Could not parse display time: ", displayTimeCacheFile)
        pass

    try:
        # Read user override time setting
        user_time = Time.load(displayTimeUserOverride)
        print("User override time: {}".format(user_time))
    except:
        print("Could not parse display time: ", displayTimeUserOverride)
        pass


if __name__ == "__main__":
    main()
