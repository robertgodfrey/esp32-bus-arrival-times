#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include "config.h"
#include "log.h"
#include "wifi_setup.h"
#include "oba_client.h"
#include "display.h"
#include "clock.h"

static unsigned long lastPoll = 0;
static TaskHandle_t fetchTaskHandle = nullptr;
static std::vector<Arrival> fetchedArrivals;
static int64_t fetchedCurrentTimeMs = 0;
static bool fetchSucceeded = false;
static volatile bool fetchComplete = false;

/**
 * fetchArrivals() blocks for as long as the network does, so it runs on its own task instead of the main loop to keep
 * the clock/display updating the whole time. This function writes the results, then sets fetchComplete=true. loop()
 * only reads the other fields after observing fetchComplete==true.
 */
static void fetchArrivalsTask(void *) {
    std::vector<Arrival> arrivals;
    int64_t currentTimeMs;
    const bool ok = fetchArrivals(arrivals, currentTimeMs);

    fetchedArrivals = std::move(arrivals);
    fetchedCurrentTimeMs = currentTimeMs;
    fetchSucceeded = ok;
    fetchComplete = true;

    fetchTaskHandle = nullptr;
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
    Serial.printf("=================================\n\n");

    displayInit();
    wifiSetup();
    clockSetup();
}

void loop() {
    const unsigned long now = millis();
    const bool shouldPoll = lastPoll == 0 || now - lastPoll >= POLL_INTERVAL_MS;

    displayLoop();

    if (fetchComplete) {
        fetchComplete = false;
        if (fetchSucceeded) {
            showArrivals(fetchedArrivals, fetchedCurrentTimeMs);
        } else {
            recordFetchFailed();
        }
    }

    if (WiFiClass::status() == WL_CONNECTED && fetchTaskHandle == nullptr && shouldPoll) {
        LOGF("rssi=%d free_heap=%u", WiFi.RSSI(), ESP.getFreeHeap());
        lastPoll = now;
        xTaskCreate(fetchArrivalsTask, "fetchArrivalsTask", 8192, nullptr, 1, &fetchTaskHandle);
    }

    delay(5);
}
