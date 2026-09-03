#pragma once

#include <vector>
#include "oba_client.h"

// Sets up the hardware needed to sound the chime -- call once from setup().
void chimeSetup();

// Checks the latest arrivals fetch for any trip that has just crossed into "N minutes away"
// (see CHIME_MINUTES_BEFORE in config.h) for the first time, and sounds the chime for it if the
// current local time falls within one of the configured chime windows (see CHIME_WINDOWS in
// config.h). Safe to call after every successful fetch -- each trip only ever chimes once,
// tracked by tripId.
void checkChimes(const std::vector<Arrival> &arrivals, int64_t serverTimeMs);
