#include "clock.h"

// Pacific time, DST-aware: starts 2nd Sunday of March, ends 1st Sunday of November
static const char *TZ_STRING = "PST8PDT,M3.2.0,M11.1.0";
static const char *NTP_SERVER = "pool.ntp.org";

void clockSetup() {
    configTzTime(TZ_STRING, NTP_SERVER);
}

bool formatCurrentTime(char *timeCharBuff, size_t bufSize) {
    tm timeInfo{};
    if (!getLocalTime(&timeInfo, 0)) {
        return false;
    }
    int hour12 = timeInfo.tm_hour % 12;
    if (hour12 == 0) {
        hour12 = 12;
    }
    snprintf(timeCharBuff, bufSize, "%d:%02d", hour12, timeInfo.tm_min);
    return true;
}
