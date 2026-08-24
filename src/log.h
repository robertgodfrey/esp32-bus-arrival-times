#pragma once

#include <Arduino.h>

#define LOGF(fmt, ...) Serial.printf("[%8lu] " fmt "\n", millis(), ##__VA_ARGS__)
