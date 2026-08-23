#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"
#include "config.h"

const String OBA_ARRIVALS_API_URL = "https://api.pugetsound.onebusaway.org/api/where/";
const String OBA_ARRIVALS_FOR_STOP_URL = OBA_ARRIVALS_API_URL + STOP_ID + ".json?key=" + OBA_API_KEY + "&minutesBefore=" + MINUTES_BEFORE;

static unsigned long lastPoll = 0;

static void onWiFiEvent(const WiFiEvent_t e, const WiFiEventInfo_t &info) {
    switch (e) {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("wifi connected");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.printf("got ip %s  dns0=%s dns1=%s\n",
                          WiFi.localIP().toString().c_str(),
                          WiFi.dnsIP(0).toString().c_str(),
                          WiFi.dnsIP(1).toString().c_str());
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.printf("disconnected reason=%d\n",
                          info.wifi_sta_disconnected.reason);
            break;
        default: break;
    }
}

static void fetchArrivals() {
    Serial.printf("rssi=%d free_heap=%u\n", WiFi.RSSI(), ESP.getFreeHeap());

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(8000);

    HTTPClient http;
    http.setConnectTimeout(8000);
    http.setTimeout(8000);

    if (!http.begin(client, OBA_ARRIVALS_FOR_STOP_URL)) {
        Serial.println("http begin failed");
        return;
    }

    Serial.println("connecting...");
    const int code = http.GET();
    Serial.println("GET returned");
    if (code != HTTP_CODE_OK) {
        Serial.printf("GET failed, code=%d (%s)\n", code, HTTPClient::errorToString(code).c_str());
        http.end();
        return;
    }

    JsonDocument doc;
    const DeserializationError err = deserializeJson(doc, http.getStream());
    http.end();

    if (err) {
        Serial.printf("json parse failed: %s\n", err.c_str());
        return;
    }

    const int64_t currentTime = doc["currentTime"] | 0LL;
    const JsonArray arrivals = doc["data"]["entry"]["arrivalsAndDepartures"];

    Serial.printf("-- %d arrivals --\n", arrivals.size());
    for (JsonObject a: arrivals) {
        const char *route = a["routeShortName"];
        const char *headsign = a["tripHeadsign"];
        int64_t predicted = a["predictedArrivalTime"] | 0LL;
        int64_t scheduled = a["scheduledArrivalTime"] | 0LL;
        int64_t arrivalTime = predicted > 0 ? predicted : scheduled;
        int64_t minutesAway = (arrivalTime - currentTime) / 60000;

        Serial.printf("route %s to %s: %lld min%s\n",
                      route, headsign, minutesAway,
                      predicted > 0 ? "" : " (scheduled)");
    }
}

void setup() {
    Serial.begin(115200);

    delay(1000);
    Serial.println("\n============ STARTUP ============");

    WiFiClass::mode(WIFI_STA);
    WiFiClass::setHostname("ESP32 Bus Time Display");
    WiFi.setSleep(false);
    WiFi.onEvent(onWiFiEvent);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void loop() {
    if (WiFiClass::status() == WL_CONNECTED && (lastPoll == 0 || millis() - lastPoll >= POLL_INTERVAL_MS)) {
        lastPoll = millis();
        fetchArrivals();
    }
}
