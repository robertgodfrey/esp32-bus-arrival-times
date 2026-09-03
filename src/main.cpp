#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include "config.h"
#include "log.h"
#include "wifi_setup.h"
#include "oba_client.h"
#include "weather_client.h"
#include "display.h"
#include "clock.h"
#include "chime.h"

static unsigned long lastArrivalsPoll = 0;
static TaskHandle_t fetchTaskHandle = nullptr;
static std::vector<Arrival> fetchedArrivals;
static int64_t fetchedServerTimeMs = 0;
static bool fetchSucceeded = false;
static volatile bool fetchComplete = false;

static unsigned long lastWeatherPoll = 0;
static TaskHandle_t weatherTaskHandle = nullptr;
static Weather fetchedWeather;
static bool weatherFetchSucceeded = false;
static volatile bool weatherFetchComplete = false;

static void fetchArrivalsTask(void *) {
    std::vector<Arrival> arrivals;
    int64_t serverTimeMs;
    const bool ok = fetchArrivals(arrivals, serverTimeMs);

    fetchedArrivals = std::move(arrivals);
    fetchedServerTimeMs = serverTimeMs;
    fetchSucceeded = ok;
    fetchComplete = true;

    fetchTaskHandle = nullptr;
    vTaskDelete(nullptr);
}

static void fetchWeatherTask(void *) {
    Weather weather{};
    const bool ok = fetchWeather(weather);

    fetchedWeather = weather;
    weatherFetchSucceeded = ok;
    weatherFetchComplete = true;

    weatherTaskHandle = nullptr;
    vTaskDelete(nullptr);
}

void setup() {
    Serial.begin(115200);

    delay(1000);
    Serial.println("\n============ STARTUP ============");
    Serial.printf("ssid=%s\n", WIFI_SSID);
    Serial.printf("stop id=%s\n", STOP_ID);
    Serial.printf("poll interval=%d ms\n", POLL_INTERVAL_MS);
    Serial.printf("minutes before=%d\n", MINUTES_BEFORE);
    Serial.printf("minutes after=%d\n", MINUTES_AFTER);
    Serial.printf("arrivals url=%s\n", obaArrivalsUrlForLogging().c_str());
    Serial.printf("weather poll interval=%d ms\n", WEATHER_POLL_INTERVAL_MS);
    Serial.printf("weather url=%s\n", weatherUrlForLogging().c_str());
    Serial.printf("=================================\n\n");

    displayInit();
    wifiSetup();
    clockSetup();
    chimeSetup();
}

void loop() {
    const unsigned long now = millis();
    const bool shouldPollArrivals = lastArrivalsPoll == 0 || now - lastArrivalsPoll >= POLL_INTERVAL_MS;
    const bool shouldPollWeather = lastWeatherPoll == 0 || now - lastWeatherPoll >= WEATHER_POLL_INTERVAL_MS;

    displayLoop();

    if (fetchComplete) {
        fetchComplete = false;
        if (fetchSucceeded) {
            showArrivals(fetchedArrivals, fetchedServerTimeMs);
        }
    }

    if (weatherFetchComplete) {
        weatherFetchComplete = false;
        if (weatherFetchSucceeded) {
            showWeather(fetchedWeather);
        }
    }

    // fetchTaskHandle/weatherTaskHandle are both checked here so the two TLS fetches never run concurrently (too much memory)
    if (WiFiClass::status() == WL_CONNECTED && fetchTaskHandle == nullptr && weatherTaskHandle == nullptr && shouldPollArrivals) {
        LOGF("rssi=%d free_heap=%u", WiFi.RSSI(), ESP.getFreeHeap());
        lastArrivalsPoll = now;
        xTaskCreate(fetchArrivalsTask, "fetchArrivalsTask", 8192, nullptr, 1, &fetchTaskHandle);
    }

    if (WiFiClass::status() == WL_CONNECTED && fetchTaskHandle == nullptr && weatherTaskHandle == nullptr && shouldPollWeather) {
        lastWeatherPoll = now;
        xTaskCreate(fetchWeatherTask, "fetchWeatherTask", 8192, nullptr, 1, &weatherTaskHandle);
    }

    delay(1000);
}
