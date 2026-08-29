#include <esp32_smartdisplay.h>
#include "display.h"
#include "clock.h"

static lv_obj_t *clockLabel;
static lv_obj_t *dateLabel;
static lv_obj_t *arrivalsLabel;
static lv_obj_t *statusLabel;
static unsigned long lastSuccessMs = 0; // 0 = never succeeded yet
static bool lastFetchFailed = false;

static std::vector<Arrival> lastArrivals;
static int64_t lastCurrentTimeMs = 0; // API's server time, at the moment of the last successful fetch
static unsigned long lastCurrentTimeMillis = 0; // millis() at that same moment, used to extrapolate forward

static void updateStatusLabel() {
    char status[48];
    if (lastSuccessMs == 0) {
        snprintf(status, sizeof(status), "%s", lastFetchFailed ? "retrying..." : "connecting...");
    } else {
        const unsigned long agoSec = (millis() - lastSuccessMs) / 1000;
        if (lastFetchFailed) {
            snprintf(status, sizeof(status), "retrying... (updated %lus ago)", agoSec);
        } else {
            snprintf(status, sizeof(status), "updated %lus ago", agoSec);
        }
    }
    lv_label_set_text(statusLabel, status);
}

static void updateClockLabel() {
    char clockText[16];
    if (formatCurrentTime(clockText, sizeof(clockText))) {
        lv_label_set_text(clockLabel, clockText);
    }
}

static void updateDateLabel() {
    char dateText[16];
    if (formatDate(dateText, sizeof(dateText))) {
        lv_label_set_text(dateLabel, dateText);
    }
}

// recomputes "minutes away" for each stored arrival based on how much time has passed since the last successful fetch,
// so the countdown keeps ticking even if subsequent fetches fail
static void renderArrivals() {
    if (lastArrivals.empty()) {
        lv_label_set_text(arrivalsLabel, "No arrivals");
        return;
    }

    const int64_t effectiveCurrentTimeMs = lastCurrentTimeMs + (millis() - lastCurrentTimeMillis);

    String text;
    for (const Arrival &arrival: lastArrivals) {
        const int64_t minutesAway = (arrival.arrivalTimeMs - effectiveCurrentTimeMs) / 60000;
        text += arrival.routeShortName + ": " + minutesAway + " min" + (arrival.predicted ? "" : " (scheduled)") + "\n";
    }
    lv_label_set_text(arrivalsLabel, text.c_str());
}

void displayInit() {
    smartdisplay_init();

    clockLabel = lv_label_create(lv_screen_active());
    lv_obj_set_width(clockLabel, lv_pct(100));
    lv_obj_set_style_text_font(clockLabel, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_align(clockLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(clockLabel, LV_ALIGN_TOP_MID, 0, 24); // 24px padding from the top edge
    lv_label_set_text(clockLabel, "00:00");

    dateLabel = lv_label_create(lv_screen_active());
    lv_obj_set_width(dateLabel, lv_pct(100));
    lv_obj_set_style_text_font(dateLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(dateLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(dateLabel, clockLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 4); // directly under the clock
    lv_label_set_text(dateLabel, "Mon, Jan 1");

    arrivalsLabel = lv_label_create(lv_screen_active());
    lv_obj_set_width(arrivalsLabel, lv_pct(100));
    lv_label_set_long_mode(arrivalsLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(arrivalsLabel, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_align(arrivalsLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(arrivalsLabel);
    lv_label_set_text(arrivalsLabel, "");

    statusLabel = lv_label_create(lv_screen_active());
    lv_obj_align(statusLabel, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_label_set_text(statusLabel, "connecting...");
}

void displayLoop() {
    static unsigned long lastTick = millis();
    const unsigned long now = millis();

    lv_tick_inc(now - lastTick);
    lastTick = now;

    // nothing on screen changes faster than once a second, so there's no reason to redraw more often than that
    static unsigned long lastRender = 0;
    if (now - lastRender >= 500) {
        lastRender = now;
        lv_timer_handler();
    }

    static unsigned long lastStatusUpdate = 0;
    if (now - lastStatusUpdate >= 1000) {
        lastStatusUpdate = now;
        updateStatusLabel();
        updateClockLabel();
        updateDateLabel();
        renderArrivals();
    }
}

void showArrivals(const std::vector<Arrival> &arrivals, const int64_t currentTimeMs) {
    lastArrivals = arrivals;
    lastCurrentTimeMs = currentTimeMs;
    lastCurrentTimeMillis = millis();
    renderArrivals();

    lastFetchFailed = false;
    lastSuccessMs = millis();
}

void recordFetchFailed() {
    lastFetchFailed = true;
}
