#!/usr/bin/python3

#
## Usage:  setclock.py HH MM
#
#           HH - hour to set
#           MM - minute to set

import sys

from SimplexProtocol import Time
if len(sys.argv) != 3:
    print("Usage:  setclock.py <HOUR> <MIN>\n")
    print("  Enter the time shown on the clock face. The Master Clock driver will")
    print("  compensate to catch the clock up to the current time.")
    exit(0)

Time(int(sys.argv[1]),int(sys.argv[2])).save("user-time.dat")
