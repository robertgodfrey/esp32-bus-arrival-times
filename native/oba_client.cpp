#include <ctime>
#include "oba_client.h"

// sample data to see how the display looks without needing a real OBA response

bool fetchArrivals(std::vector<Arrival> &arrivals, int64_t &currentTimeMs) {
    currentTimeMs = static_cast<int64_t>(time(nullptr)) * 1000;

    arrivals = {
        {"1 Line", "Seattle", currentTimeMs + 3 * 60000, true},
        {"1 Line", "Seattle", currentTimeMs + 12 * 60000, false},
    };
    return true;
}

String obaArrivalsUrlForLogging() {
    return "nothing to see here";
}
