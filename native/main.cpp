#include <chrono>
#include <thread>
#include <vector>
#include "display.h"
#include "clock.h"
#include "oba_client.h"

int main() {
    displayInit();
    clockSetup();

    // one fake fetch at startup -- renderArrivals() already recomputes the
    // "minutes away" countdown every second on its own, so this is enough to
    // see the live countdown behavior working, same as on the real device
    std::vector<Arrival> arrivals;
    int64_t currentTimeMs;
    if (fetchArrivals(arrivals, currentTimeMs)) {
        showArrivals(arrivals, currentTimeMs);
    }

    while (true) {
        displayLoop();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
