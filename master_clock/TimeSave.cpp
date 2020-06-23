#include <FS.h>
#include <LittleFS.h>
#include <time.h>

#include "TimeSave.h"
#include "clock_generic.h"
#include "console.h"

#define POWERLOSS_FILE "clockface.txt"

static int prev_time = -1;

/** Note: I save multiple time values in the file and then only keep then
last time added. I do this on the assumption that appending to the file
will not cause extra wear-leveling that overwriting will. But I don't know
enought about LittleFS to know if this is really true. Maybe this is a
waste of time.  And it does take time.  Mounting the FS or something in here
takes many milliseconds. Probably we should yield in here or something.
*/


// Get last displayed walltime in seconds
int readTime()
{
  if (!LittleFS.begin()) {
    p("LittleFS mount failed\n");
    return -1;
  }

  File file = LittleFS.open(POWERLOSS_FILE, "r");
  if (!file) {
    p("File read failed: " POWERLOSS_FILE "\n");
  }

  if (file.size() > 20) {
        // read the last 20 bytes of the file.
        file.seek(file.size() - 20);
        // skip the next number, likely to be bad
        file.parseInt();
  }

  int t = -1;
  while (file.available()) {
        // TODO: better value validation
        auto rt = file.parseInt() - 1;
        if (rt >= 0 && rt < MAX_TIME/60) t = rt;
  }
  file.close();
  LittleFS.end();

  prev_time = t;
  return t * 60;
}

// Save current displayed walltime to flash
bool saveTime()
{
  // Note: We record the time in minutes since we do not have a second-hand
  auto t = getWallTime() / 60;
  if (t == prev_time) return false;

  if (!LittleFS.begin()) {
    p("LittleFS mount failed\n");
    return false;
  }

  bool append = true;
  File file = LittleFS.open(POWERLOSS_FILE, "a");
  if (!file) {
    p("File append failed: " POWERLOSS_FILE "\n");
  }
  if (!file || file.size() > 4000) {
        append = false;
        if (file) file.close();
        file = LittleFS.open(POWERLOSS_FILE, "w");
        if (!file) {
                p("File save failed: " POWERLOSS_FILE "\n");
                return false;
        }
  }

  // We write t+1 because parseInt("bogus")==0
  bool saved = file.println(t+1);

  if (saved) {
    file.close();
//     p("<save[%c] %d>", append?'a':'w', t);
    prev_time = t;
//     auto verify = readTime() / 60;
//     if (t != verify) p("wtf? read-time doesn't match: %d != %d\n", t, verify);
  } else {
    p("<save-failed>");
    file.close();
  }
  LittleFS.end();

  return saved;
}