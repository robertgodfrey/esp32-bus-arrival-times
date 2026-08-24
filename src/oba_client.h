#pragma once

#include <Arduino.h>
#include <vector>

struct Arrival {
    String routeShortName;
    String headsign;
    int64_t arrivalTimeMs; // absolute epoch ms -- predicted if available, else scheduled
    bool predicted;
};

// Fetches current arrivals for the configured stop/route.
// On success, returns true, fills `arrivals`, and sets `currentTimeMs` to the
// API's reported server time at fetch -- callers can use it as an anchor to
// keep computing accurate "minutes away" values as time passes.
bool fetchArrivals(std::vector<Arrival> &arrivals, int64_t &currentTimeMs);

// The request URL, without the API key, for startup logging.
String obaArrivalsUrlForLogging();
