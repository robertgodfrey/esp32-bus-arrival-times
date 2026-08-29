#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "weather_client.h"
#include "config.h"
#include "log.h"

static const String WEATHER_URL =
    String("https://api.open-meteo.com/v1/forecast?latitude=") + LATITUDE + "&longitude=" + LONGITUDE +
    "&forecast_days=1&current=temperature_2m&daily=temperature_2m_max,temperature_2m_min,weather_code" +
    "&temperature_unit=fahrenheit&timezone=auto";

/*
https://api.open-meteo.com/v1/forecast?latitude=47.6062&longitude=-122.3321&current=temperature_2m&daily=temperature_2m_max,temperature_2m_min,weather_code&temperature_unit=fahrenheit&timezone=auto
*/

String weatherUrlForLogging() {
    return WEATHER_URL;
}

// see the WMO weather code table: https://open-meteo.com/en/docs
static WeatherCondition conditionFromCode(const int code) {
    if (code == 0) {
        return WeatherCondition::Clear;
    }
    if (code == 1) {
        return WeatherCondition::MainlyClear;
    }
    if (code == 2) {
        return WeatherCondition::PartlyCloudy;
    }
    if (code == 3) {
        return WeatherCondition::Overcast;
    }
    if (code == 45 || code == 48) {
        return WeatherCondition::Fog;
    }
    if (code >= 51 && code <= 57) {
        return WeatherCondition::Drizzle;
    }
    if (code >= 61 && code <= 67) {
        return WeatherCondition::Rain;
    }
    if (code >= 80 && code <= 82) {
        return WeatherCondition::RainShowers;
    }
    if ((code >= 71 && code <= 77) || code == 85 || code == 86) {
        return WeatherCondition::Snow;
    }
    return WeatherCondition::Thunderstorm; // 95-99
}

bool fetchWeather(Weather &weather) {
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(8000);

    HTTPClient http;
    http.setConnectTimeout(8000);
    http.setTimeout(8000);

    if (!http.begin(client, WEATHER_URL)) {
        LOGF("weather: http begin failed");
        return false;
    }

    // open-meteo responds with Transfer-Encoding: chunked, and getStream() below hands
    // deserializeJson() the raw un-dechunked bytes -- useHTTP10 avoids chunked encoding
    // entirely so the stream is plain JSON
    http.useHTTP10(true);

    LOGF("fetching weather...");
    const unsigned long start = millis();
    const int code = http.GET();
    LOGF("weather fetch done in %lums, code=%d", millis() - start, code);
    if (code != HTTP_CODE_OK) {
        LOGF("weather GET failed, code=%d (%s)", code, HTTPClient::errorToString(code).c_str());
        http.end();
        return false;
    }

    JsonDocument doc;
    const DeserializationError err = deserializeJson(doc, http.getStream());
    http.end();

    if (err) {
        LOGF("weather json parse failed: %s", err.c_str());
        return false;
    }

    weather.currentTempF = doc["current"]["temperature_2m"] | 0.0f;
    weather.highTempF = doc["daily"]["temperature_2m_max"][0] | 0.0f;
    weather.lowTempF = doc["daily"]["temperature_2m_min"][0] | 0.0f;
    weather.condition = conditionFromCode(doc["daily"]["weather_code"][0] | 0);

    LOGF("weather: %.0fF (H%.0f L%.0f) condition=%d",
         weather.currentTempF, weather.highTempF, weather.lowTempF, static_cast<int>(weather.condition));

    return true;
}
