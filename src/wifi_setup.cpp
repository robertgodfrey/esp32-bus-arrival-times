#include <WiFi.h>
#include "wifi_setup.h"
#include "config.h"

// runs on the small-stack WiFi system event task, don't use LOGF or printf here
static void onWiFiEvent(const WiFiEvent_t e, const WiFiEventInfo_t &info) {
    Serial.print('[');
    Serial.print(millis());
    Serial.print("] ");
    switch (e) {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("wifi connected");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("got ip ");
            Serial.print(WiFi.localIP());
            Serial.print("  dns0=");
            Serial.print(WiFi.dnsIP(0));
            Serial.print(" dns1=");
            Serial.println(WiFi.dnsIP(1));
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.print("disconnected reason=");
            Serial.println(info.wifi_sta_disconnected.reason);
            break;
        default:
            Serial.println();
            break;
    }
}

void wifiSetup() {
    WiFiClass::setHostname("ESP32 Display");
    WiFiClass::mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.onEvent(onWiFiEvent);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
}
