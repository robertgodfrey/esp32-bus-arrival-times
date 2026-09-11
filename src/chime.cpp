#include <algorithm>
#include "chime.h"
#include "chime_tone.h"
#include "clock.h"
#include "config.h"

namespace {
    struct ChimeState {
        String tripId;
        bool sounded;
        bool hasPriorReading;
        int64_t lastMinutesAway;
    };

    std::vector<ChimeState> chimeStates;

    ChimeState &stateFor(const String &tripId) {
        for (ChimeState &state: chimeStates) {
            if (state.tripId == tripId) {
                return state;
            }
        }
        chimeStates.push_back({tripId, false, false, 0});
        return chimeStates.back();
    }

    // drops tracked trips that have fallen out of the current arrivals window
    void forgetStaleTrips(const std::vector<Arrival> &arrivals) {
        chimeStates.erase(std::remove_if(chimeStates.begin(), chimeStates.end(),
                                         [&](const ChimeState &state) {
                                             for (const Arrival &arrival: arrivals) {
                                                 if (arrival.tripId == state.tripId) {
                                                     return false;
                                                 }
                                             }
                                             return true;
                                         }),
                          chimeStates.end());
    }

    bool isWithinChimeWindow() {
        int hour, minute;
        if (!getCurrentHourMinute(hour, minute)) {
            return false; // time not synced yet -- don't chime
        }

        const int nowMinutes = hour * 60 + minute;
        for (const auto &window: CHIME_WINDOWS) {
            if (nowMinutes >= window[0] && nowMinutes <= window[1]) {
                return true;
            }
        }
        return false;
    }
}

void chimeSetup() {
    chimeToneSetup();
}

void checkChimes(const std::vector<Arrival> &arrivals, const int64_t serverTimeMs) {
    for (const Arrival &arrival: arrivals) {
        ChimeState &state = stateFor(arrival.tripId);
        const int64_t minutesAway = (arrival.arrivalTimeMs - serverTimeMs) / 60000;

        const bool shouldSoundChime = !state.sounded
                                      && state.hasPriorReading
                                      && state.lastMinutesAway > CHIME_MINUTES_BEFORE
                                      && minutesAway <= CHIME_MINUTES_BEFORE
                                      && isWithinChimeWindow();

        if (shouldSoundChime) {
            playChimeTone();
            state.sounded = true;
        }

        state.lastMinutesAway = minutesAway;
        state.hasPriorReading = true;
    }
    forgetStaleTrips(arrivals);
}
