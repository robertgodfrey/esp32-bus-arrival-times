#include "oba_client.h"
#include "config.h"

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
