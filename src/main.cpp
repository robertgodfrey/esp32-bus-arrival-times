#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <esp32_smartdisplay.h>
#include "secrets.h"
#include "config.h"

const String OBA_ARRIVALS_API_URL = "https://api.pugetsound.onebusaway.org/api/where/arrivals-and-departures-for-stop/";
const String OBA_ARRIVALS_FOR_STOP_URL = OBA_ARRIVALS_API_URL + STOP_ID + ".json";
const String URL_PARAMS = String("?key=") + OBA_API_KEY + "&minutesBefore=" + MINUTES_BEFORE + "&minutesAfter=" + MINUTES_AFTER;

static unsigned long lastPoll = 0;
static lv_obj_t *arrivalsLabel;

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
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(8000);

    HTTPClient http;
    http.setConnectTimeout(8000);
    http.setTimeout(8000);

    if (!http.begin(client, OBA_ARRIVALS_FOR_STOP_URL + URL_PARAMS)) {
        Serial.println("http begin failed");
        return;
    }

    Serial.println("fetching arrivals...");
    const unsigned long start = millis();
    const int code = http.GET();
    Serial.printf("fetch done in %lums, code=%d\n", millis() - start, code);
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
    String display;
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

        display += String(route) + " to " + headsign + ": " + minutesAway + " min\n";
    }
    lv_label_set_text(arrivalsLabel, display.c_str());
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
    Serial.printf("arrivals url: %s\n", OBA_ARRIVALS_FOR_STOP_URL.c_str());
    Serial.printf("=================================\n\n");

    smartdisplay_init();
    arrivalsLabel = lv_label_create(lv_screen_active());
    lv_obj_set_width(arrivalsLabel, lv_pct(100));
    lv_label_set_long_mode(arrivalsLabel, LV_LABEL_LONG_WRAP);
    lv_label_set_text(arrivalsLabel, "connecting...");

    WiFiClass::mode(WIFI_STA);
    WiFiClass::setHostname("ESP32 Bus Time Display");
    WiFi.setSleep(false);
    WiFi.onEvent(onWiFiEvent);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void loop() {
    static unsigned long lastTick = millis();
    const unsigned long now = millis();
    lv_tick_inc(now - lastTick);
    lastTick = now;
    lv_timer_handler();

    if (WiFiClass::status() == WL_CONNECTED && (lastPoll == 0 || now - lastPoll >= POLL_INTERVAL_MS)) {
        Serial.printf("rssi=%d free_heap=%u\n", WiFi.RSSI(), ESP.getFreeHeap());
        lastPoll = now;
        fetchArrivals();
    }

    delay(5);
}
