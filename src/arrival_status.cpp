#include "oba_client.h"
#include "config.h"

// platform-independent (no network/hardware), so it's its own file rather than living in
// oba_client.cpp -- that file is swapped per-platform, this logic isn't

ArrivalStatus arrivalStatus(const Arrival &arrival) {
    if (!arrival.predicted) {
        return ArrivalStatus::Scheduled;
    }
    const int64_t deltaMs = arrival.arrivalTimeMs - arrival.scheduledTimeMs;
    if (deltaMs > ARRIVAL_STATUS_THRESHOLD_MS) {
        return ArrivalStatus::Late;
    }
    if (deltaMs < -ARRIVAL_STATUS_THRESHOLD_MS) {
        return ArrivalStatus::Early;
    }
    return ArrivalStatus::OnTime;
}
