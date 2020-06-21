/**
 * NTP service state machine.
 *
 * Syncrhonizes time with NTP server once per hour.
 * Allows user to initiate one-off synchronization
 * as needed.
 */

//_____________________________________________________________________
//                                                             INCLUDES
#include "Arduino.h"
#include "TimeService.h"
#include "clock_generic.h"

void NtpSetup()
{
        TimeService::begin();
}

void NtpService() {
        if (TimeService::isStale()) {
                // flash the LED 4 times if our time sync isn't working
                showActivity(4);
        }
}