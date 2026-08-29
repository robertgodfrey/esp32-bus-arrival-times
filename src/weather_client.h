#pragma once

#include <Arduino.h>

enum class WeatherCondition {
    Clear,
    MainlyClear,
    PartlyCloudy,
    Overcast,
    Fog,
    Drizzle,
    Rain,
    RainShowers,
    Snow,
    Thunderstorm,
};

struct Weather {
    float currentTempF;
    float highTempF;
    float lowTempF;
    WeatherCondition condition; // today's overall condition, not just the current moment
};

// Fetches current temp + today's high/low/condition for the configured location.
// Returns true and fills `weather` on success; returns false on any failure.
bool fetchWeather(Weather &weather);

// The request URL, for startup logging.
String weatherUrlForLogging();
