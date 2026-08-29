#include "weather_client.h"

// sample data

bool fetchWeather(Weather &weather) {
    weather.currentTempF = 68.0f;
    weather.highTempF = 72.0f;
    weather.lowTempF = 58.0f;
    weather.condition = WeatherCondition::PartlyCloudy;
    return true;
}

String weatherUrlForLogging() {
    return "nothing to see here";
}
