#include <ctime>
#include "oba_client.h"

// sample data to see how the display looks without needing a real OBA response

bool fetchArrivals(std::vector<Arrival> &arrivals, int64_t &currentTimeMs) {
    currentTimeMs = static_cast<int64_t>(time(nullptr)) * 1000;

    arrivals = {
        // predicted 3 min later than scheduled -- exercises the "Late" status color
        {"trip1", "1 Line", "Seattle", currentTimeMs + 4 * 60000, currentTimeMs + 1 * 60000, true},
        // no real-time prediction -- exercises the "Scheduled" status color
        {"trip2", "1 Line", "Seattle", currentTimeMs + 12 * 60000, currentTimeMs + 12 * 60000, false},
    };
    return true;
}

String obaArrivalsUrlForLogging() {
    return "nothing to see here";
}
