#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "oba_client.h"
#include "config.h"
#include "log.h"

static const String OBA_ARRIVALS_API_URL = "https://api.pugetsound.onebusaway.org/api/where/arrivals-and-departures-for-stop/";
static const String OBA_ARRIVALS_FOR_STOP_URL = OBA_ARRIVALS_API_URL + STOP_ID + ".json";
static const String URL_PARAMS = String("?key=") + OBA_API_KEY + "&minutesBefore=" + MINUTES_BEFORE + "&minutesAfter=" + MINUTES_AFTER;

String obaArrivalsUrlForLogging() {
    return OBA_ARRIVALS_FOR_STOP_URL;
}

bool fetchArrivals(std::vector<Arrival> &arrivals, int64_t &currentTimeMs) {
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(8000);

    HTTPClient http;
    http.setConnectTimeout(8000);
    http.setTimeout(8000);
    http.useHTTP10(true);

    if (!http.begin(client, OBA_ARRIVALS_FOR_STOP_URL + URL_PARAMS)) {
        LOGF("http begin failed");
        return false;
    }

    LOGF("fetching arrivals...");
    const unsigned long start = millis();
    const int code = http.GET();
    LOGF("fetch done in %lums, code=%d", millis() - start, code);
    if (code != HTTP_CODE_OK) {
        LOGF("GET failed, code=%d (%s)", code, HTTPClient::errorToString(code).c_str());
        http.end();
        return false;
    }

    // the OBA response includes a large "references" section that nothing here reads. filtering it out keeps the parsed
    // DOM to just the handful of fields actually used
    JsonDocument filter;
    filter["currentTime"] = true;
    JsonObject filterArrival = filter["data"]["entry"]["arrivalsAndDepartures"][0].to<JsonObject>();
    filterArrival["tripId"] = true;
    filterArrival["routeShortName"] = true;
    filterArrival["tripHeadsign"] = true;
    filterArrival["routeId"] = true;
    filterArrival["predictedArrivalTime"] = true;
    filterArrival["scheduledArrivalTime"] = true;

    JsonDocument doc;
    const DeserializationError err = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
    http.end();

    if (err) {
        LOGF("json parse failed: %s", err.c_str());
        return false;
    }

    currentTimeMs = doc["currentTime"] | 0LL;
    const JsonArray jsonArrivals = doc["data"]["entry"]["arrivalsAndDepartures"];

    LOGF("-- %d arrivals --", jsonArrivals.size());
    arrivals.clear();

    for (JsonObject a: jsonArrivals) {
        const char *tripId = a["tripId"];
        const char *routeShortName = a["routeShortName"];
        const char *headsign = a["tripHeadsign"];

        if (a["routeId"] != ROUTE_ID) {
            LOGF("skipping route %s to %s", routeShortName, headsign);
            continue;
        }

        int64_t predicted = a["predictedArrivalTime"] | 0LL;
        int64_t scheduled = a["scheduledArrivalTime"] | 0LL;
        int64_t arrivalTime = predicted > 0 ? predicted : scheduled;

        LOGF("route %s to %s: %lld min%s", routeShortName, headsign,
             (arrivalTime - currentTimeMs) / 60000, predicted > 0 ? "" : " (scheduled)");

        arrivals.push_back({tripId, routeShortName, headsign, arrivalTime, scheduled, predicted > 0});
    }
    return true;
}
