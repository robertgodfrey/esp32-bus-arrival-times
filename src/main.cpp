#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include "config.h"
#include "log.h"
#include "wifi_setup.h"
#include "oba_client.h"
#include "display.h"

static unsigned long lastPoll = 0;

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
}

void loop() {
    displayLoop();

    const unsigned long now = millis();
    if (WiFiClass::status() == WL_CONNECTED && (lastPoll == 0 || now - lastPoll >= POLL_INTERVAL_MS)) {
        LOGF("rssi=%d free_heap=%u", WiFi.RSSI(), ESP.getFreeHeap());

        lastPoll = now;
        std::vector<Arrival> arrivals;
        int64_t currentTimeMs;

        if (fetchArrivals(arrivals, currentTimeMs)) {
            showArrivals(arrivals, currentTimeMs);
        } else {
            reportFetchFailed();
        }
    }

    delay(5);
}
