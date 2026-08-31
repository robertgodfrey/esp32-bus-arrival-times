#pragma once

#include <Arduino.h>
#include <vector>

struct Arrival {
    String tripId; // stable per scheduled trip -- used to dedupe chime alerts across polls
    String routeShortName;
    String headsign;
    int64_t arrivalTimeMs; // absolute epoch ms -- predicted if available, else scheduled
    int64_t scheduledTimeMs; // absolute epoch ms -- always the schedule, used to compute ArrivalStatus
    bool predicted;
};

enum class ArrivalStatus {
    Scheduled, // no real-time prediction available at all
    OnTime,
    Early,
    Late,
};

// compares predicted vs scheduled (see ARRIVAL_STATUS_THRESHOLD_MS in config.h) -- always
// Scheduled if there's no real-time prediction for this arrival
ArrivalStatus arrivalStatus(const Arrival &arrival);

// Fetches current arrivals for the configured stop/route.
// On success, returns true, fills `arrivals`, and sets `currentTimeMs` to the
// API's reported server time at fetch -- callers can use it as an anchor to
// keep computing accurate "minutes away" values as time passes.
bool fetchArrivals(std::vector<Arrival> &arrivals, int64_t &currentTimeMs);

// The request URL, without the API key, for startup logging.
String obaArrivalsUrlForLogging();
