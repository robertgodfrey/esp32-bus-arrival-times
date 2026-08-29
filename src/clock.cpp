#include "clock.h"
#include "log.h"

// Pacific time, DST-aware: starts 2nd Sunday of March, ends 1st Sunday of November
static auto TZ_STRING = "PST8PDT,M3.2.0,M11.1.0";
static auto *NTP_SERVER = "pool.ntp.org";

void clockSetup() {
    configTzTime(TZ_STRING, NTP_SERVER);
}

bool formatCurrentTime(char *timeCharBuff, const size_t bufSize) {
    tm timeInfo{};
    if (!getLocalTime(&timeInfo, 0)) {
        // don't spam the log every second -- once every 10s is plenty to confirm this is the cause
        static unsigned long lastWarnMs = 0;
        if (millis() - lastWarnMs > 10000) {
            lastWarnMs = millis();
            LOGF("NTP not synced yet");
        }
        return false;
    }
    int hour12 = timeInfo.tm_hour % 12;
    if (hour12 == 0) {
        hour12 = 12;
    }
    snprintf(timeCharBuff, bufSize, "%d:%02d", hour12, timeInfo.tm_min);
    return true;
}

bool formatDate(char *dateCharBuff, const size_t bufSize) {
    tm dateInfo{};
    if (!getLocalTime(&dateInfo, 0)) {
        return false;
    }
    strftime(dateCharBuff, bufSize, "%a, %b %d", &dateInfo);
    return true;
}
