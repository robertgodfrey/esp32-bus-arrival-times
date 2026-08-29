#include <ctime>
#include "clock.h"

void clockSetup() {
    // system time is already available
}

bool formatCurrentTime(char *timeCharBuff, const size_t bufSize) {
    const time_t now = time(nullptr);
    tm timeInfo{};
    localtime_r(&now, &timeInfo);

    int hour12 = timeInfo.tm_hour % 12;
    if (hour12 == 0) {
        hour12 = 12;
    }
    snprintf(timeCharBuff, bufSize, "%d:%02d", hour12, timeInfo.tm_min);
    return true;
}

bool formatDate(char *dateCharBuff, const size_t bufSize) {
    const time_t now = time(nullptr);
    tm dateInfo{};
    localtime_r(&now, &dateInfo);

    strftime(dateCharBuff, bufSize, "%a, %b %d", &dateInfo);
    return true;
}
