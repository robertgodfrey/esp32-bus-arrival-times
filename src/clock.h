#pragma once

#include <Arduino.h>

// starts NTP sync -- call once from setup(), any time after WiFi.begin()
void clockSetup();

// fills buf with the current local time (e.g. "3:47").
// returns false if time hasn't synced yet (buf left untouched in that case)
bool formatCurrentTime(char *timeCharBuff, size_t bufSize);
