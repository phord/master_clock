// Time service
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval

class TimeService
{
public:
        static void begin();

        // How long has it been since we've updated our official time (seconds)
        static time_t timeSinceUpdate();

        // Have we ever heard from a time TimeService
        static bool hasBeenSynced();

        // Do we consider the current time to be unreliable
        static bool isStale();

        // Return the current localtime
        static const tm* localtime();

        // Set the system time from some authoritative source
        void setTime(time_t epoch);
};