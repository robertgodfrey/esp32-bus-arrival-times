#include <algorithm>
#include "chime.h"
#include "chime_tone.h"
#include "clock.h"
#include "config.h"

namespace {
    struct ChimeState {
        String tripId;
        bool sounded;
    };

    std::vector<ChimeState> chimeStates;

    bool &soundedFlagFor(const String &tripId) {
        for (ChimeState &state: chimeStates) {
            if (state.tripId == tripId) {
                return state.sounded;
            }
        }
        chimeStates.push_back({tripId, false});
        return chimeStates.back().sounded;
    }

    // drops tracked trips that have fallen out of the current arrivals window (arrived and aged
    // out, or cancelled) -- otherwise this list would grow for as long as the device stays on
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
        bool &sounded = soundedFlagFor(arrival.tripId);
        if (sounded) {
            continue;
        }

        const int64_t minutesAway = (arrival.arrivalTimeMs - serverTimeMs) / 60000;
        if (minutesAway >= 0 && minutesAway <= CHIME_MINUTES_BEFORE && isWithinChimeWindow()) {
            playChimeTone();
            sounded = true;
        }
    }
    forgetStaleTrips(arrivals);
}
