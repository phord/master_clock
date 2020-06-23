// Time service
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
#include <coredecls.h>                  // settimeofday_cb()

#include "TimeService.h"
#include "clock_generic.h"
#include "console.h"

// Missing this in the time.h include I'm using.
extern "C" int settimeofday(const struct timeval *, const struct timezone *);


#define STALE_TIME    (5*60*60)         // Stale is when we have no updates for 5 hours

static time_t updated = 0;

static void settime_cb() {
        // everything is allowed in this function

        static unsigned long firstNtp = 0 ;    ///< First sync time

        auto now = time(nullptr);
        updated = now;

        unsigned hh = (now % 86400L) / 3600 ;
        unsigned mm = (now  % 3600) / 60;
        unsigned ss = now % 60;

        p("NTP time = %02u:%02u:%02u UTC\n", hh , mm , ss ); // print the time
}

void TimeService::begin()
{
        // install callback - called when settimeofday is called (by SNTP or us)
        // once enabled (by DHCP), SNTP is updated every hour
        settimeofday_cb(settime_cb);
}


// How long has it been since we've updated our official TimeService::time (seconds)
time_t TimeService::timeSinceUpdate()
{
        if (!updated && hasBeenSynced()) {
                // We didn't see the update, but the clock is running. Maybe we were slow to start.
                updated = time(nullptr);
        }

        if (!updated) { return -1; }
        return time(nullptr) - updated;
}

// Have we ever heard from a time TimeService
bool TimeService::hasBeenSynced()
{
        return time(nullptr) > 1E7;
}

// Do we consider the current time to be unreliable
bool TimeService::isStale()
{
        return !hasBeenSynced() || timeSinceUpdate() > STALE_TIME;
}

// Return the current localtime
time_t TimeService::localtime()
{
        auto now = time(nullptr);
        auto tm = ::localtime(&now);

        // Convert localtime structure to epoch time representation
        time_t local = tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;
        return local ;
}

// Set the system time from some authoritative source
void TimeService::setTime(time_t epoch)
{
        // Note: This will cause a callback to settime_cb()

        timeval tv = { epoch, 0 };
        settimeofday(&tv, nullptr);
}
