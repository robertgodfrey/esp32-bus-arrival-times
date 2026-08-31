#ifdef NATIVE_SIMULATOR
#include "lvgl.h"
#include "drivers/sdl/lv_sdl_window.h"
#else
#include <esp32_smartdisplay.h>
#endif
#include "display.h"
#include "clock.h"
#include "chime.h"
#include "config.h"

static lv_obj_t *clockLabel;
static lv_obj_t *dateLabel;
static lv_obj_t *currentTempLabel;
static lv_obj_t *highLowLabel;
static lv_obj_t *arrivalRowsContainer; // cleaned and rebuilt each render -- see renderArrivals()
static lv_obj_t *statusLabel;
static unsigned long lastSuccessMs = 0; // 0 = never succeeded yet
static bool lastFetchFailed = false;

static bool hasWeather = false;
static Weather lastWeather;

static std::vector<Arrival> lastArrivals;
static int64_t lastCurrentTimeMs = 0; // API's server time, at the moment of the last successful fetch
static unsigned long lastCurrentTimeMillis = 0; // millis() at that same moment, used to extrapolate forward

static void updateStatusLabel() {
    char status[48];
    if (lastSuccessMs == 0) {
        snprintf(status, sizeof(status), "%s", lastFetchFailed ? "retrying..." : "connecting...");
    } else {
        const unsigned long agoMins = (millis() - lastSuccessMs) / 60000;
        if (agoMins == 0) {
            snprintf(status, sizeof(status), "Updated less than a minute ago");
        } else {
            snprintf(status, sizeof(status), "Updated %lu min ago", agoMins);
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

static void updateWeatherLabel() {
    if (!hasWeather) {
        return;
    }
    char currentText[8];
    snprintf(currentText, sizeof(currentText), "%.0f°", lastWeather.currentTempF);
    lv_label_set_text(currentTempLabel, currentText);

    char highLowText[16];
    snprintf(highLowText, sizeof(highLowText), "%.0f° / %.0f°", lastWeather.lowTempF, lastWeather.highTempF);
    lv_label_set_text(highLowLabel, highLowText);
}

// matches OneBusAway's own convention (green/blue/red/gray), not a universal standard -- chosen
// because this project talks directly to pugetsound.onebusaway.org, not because one exists
static lv_color_t statusColor(const ArrivalStatus status) {
    switch (status) {
        case ArrivalStatus::OnTime: return lv_color_hex(0x43A047);
        case ArrivalStatus::Late: return lv_color_hex(0x1E88E5);
        case ArrivalStatus::Early: return lv_color_hex(0xE53935);
        case ArrivalStatus::Scheduled: return lv_color_hex(0x78909C);
    }
    return lv_color_hex(0x78909C);
}

// plain colored text, no pill -- deliberately a different visual form than addBadge() below, so
// the route name and the arrival time/status read as two different kinds of thing at a glance
// instead of two same-shaped badges that only differ by color
static void addRouteLabel(lv_obj_t *parent, const char *text, const lv_color_t color) {
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_label_set_text(label, text);
}

static void addBadge(lv_obj_t *parent, const char *text, const lv_color_t bgColor) {
    lv_obj_t *badge = lv_obj_create(parent);
    lv_obj_remove_style_all(badge);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(badge, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(badge, bgColor, 0);
    lv_obj_set_style_bg_opa(badge, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(badge, 8, 0);
    lv_obj_set_style_pad_hor(badge, 14, 0);
    lv_obj_set_style_pad_ver(badge, 4, 0);

    lv_obj_t *label = lv_label_create(badge);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_text(label, text);
}

// recomputes "minutes away" for each stored arrival based on how much time has passed since the last successful fetch,
// so the countdown keeps ticking even if subsequent fetches fail. rebuilds the row widgets every call rather than
// patching text in place -- simpler than tracking per-row label pointers, and cheap at the handful of rows this ever
// holds
static void renderArrivals() {
    const int64_t effectiveCurrentTimeMs = lastCurrentTimeMs + (millis() - lastCurrentTimeMillis);
    checkChimes(lastArrivals, effectiveCurrentTimeMs);

    lv_obj_clean(arrivalRowsContainer);

    if (lastArrivals.empty()) {
        lv_obj_t *emptyLabel = lv_label_create(arrivalRowsContainer);
        lv_obj_set_style_text_font(emptyLabel, &lv_font_montserrat_18, 0);
        lv_label_set_text(emptyLabel, "No arrivals");
        return;
    }

    for (const Arrival &arrival: lastArrivals) {
        lv_obj_t *row = lv_obj_create(arrivalRowsContainer);
        lv_obj_remove_style_all(row);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(row, 8, 0);

        addRouteLabel(row, arrival.routeShortName.c_str(), lv_color_hex(0x9C182F));

        const int64_t minutesAway = (arrival.arrivalTimeMs - effectiveCurrentTimeMs) / 60000;
        String timeText;
        if (minutesAway == 0) {
            timeText = "NOW";
        } else {
            timeText = String(minutesAway) + " min";
        }
        addBadge(row, timeText.c_str(), statusColor(arrivalStatus(arrival)));
    }
}

void displayInit() {
#ifdef NATIVE_SIMULATOR
    lv_init();
    lv_display_t *disp = lv_sdl_window_create(240, 320);
    lv_sdl_window_set_title(disp, "bus-display simulator");
#else
    smartdisplay_init();
#endif

    // -- time/date group --
    lv_obj_t *timeGroup = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(timeGroup);
    lv_obj_clear_flag(timeGroup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(timeGroup, lv_pct(100));
    lv_obj_set_height(timeGroup, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(timeGroup, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(timeGroup, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(timeGroup, 4, 0);
    lv_obj_align(timeGroup, LV_ALIGN_TOP_MID, 0, 24); // 24px padding from the top edge

    clockLabel = lv_label_create(timeGroup);
    lv_obj_set_width(clockLabel, lv_pct(100));
    lv_obj_set_style_text_font(clockLabel, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_align(clockLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(clockLabel, "00:00");

    dateLabel = lv_label_create(timeGroup);
    lv_obj_set_width(dateLabel, lv_pct(100));
    lv_obj_set_style_text_font(dateLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(dateLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(dateLabel, "Mon, Jan 1");

    // -- weather group --
    lv_obj_t *weatherGroup = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(weatherGroup);
    lv_obj_clear_flag(weatherGroup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(weatherGroup, lv_pct(100));
    lv_obj_set_height(weatherGroup, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(weatherGroup, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(weatherGroup, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(weatherGroup, 2, 0);
    lv_obj_align_to(weatherGroup, timeGroup, LV_ALIGN_OUT_BOTTOM_MID, 0, 6); // directly under the time group

    // icon + current temp share a row; high/low sits on its own row below
    lv_obj_t *currentRow = lv_obj_create(weatherGroup);
    lv_obj_remove_style_all(currentRow);
    lv_obj_clear_flag(currentRow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(currentRow, LV_SIZE_CONTENT, 44);
    lv_obj_set_flex_flow(currentRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(currentRow, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(currentRow, 8, 0);

    // placeholder -- weather icon design pending, this just holds the layout space
    lv_obj_t *weatherIcon = lv_obj_create(currentRow);
    lv_obj_remove_style_all(weatherIcon);
    lv_obj_clear_flag(weatherIcon, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(weatherIcon, 44, 44);
    lv_obj_set_style_bg_color(weatherIcon, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_bg_opa(weatherIcon, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(weatherIcon, 8, 0);

    currentTempLabel = lv_label_create(currentRow);
    lv_obj_set_style_text_font(currentTempLabel, &lv_font_montserrat_18, 0);
    lv_label_set_text(currentTempLabel, "");

    highLowLabel = lv_label_create(weatherGroup);
    lv_obj_set_style_text_font(highLowLabel, &lv_font_montserrat_14, 0);
    lv_label_set_text(highLowLabel, "");

    // -- arrivals group --
    lv_obj_t *arrivalsGroup = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(arrivalsGroup);
    lv_obj_clear_flag(arrivalsGroup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(arrivalsGroup, lv_pct(100));
    lv_obj_set_height(arrivalsGroup, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(arrivalsGroup, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(arrivalsGroup, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(arrivalsGroup, 10, 0);
    lv_obj_align_to(arrivalsGroup, weatherGroup, LV_ALIGN_OUT_BOTTOM_MID, 0, 14); // directly under the weather group

    lv_obj_t *separator = lv_obj_create(arrivalsGroup);
    lv_obj_remove_style_all(separator);
    lv_obj_clear_flag(separator, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(separator, lv_pct(80), 2);
    lv_obj_set_style_bg_color(separator, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_bg_opa(separator, LV_OPA_COVER, 0);

    lv_obj_t *stopHeaderRow = lv_obj_create(arrivalsGroup);
    lv_obj_remove_style_all(stopHeaderRow);
    lv_obj_clear_flag(stopHeaderRow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(stopHeaderRow, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(stopHeaderRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(stopHeaderRow, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(stopHeaderRow, 6, 0);

    // placeholder -- bus icon design pending, this just holds the layout space
    lv_obj_t *busIconContainer = lv_obj_create(stopHeaderRow);
    lv_obj_remove_style_all(busIconContainer);
    lv_obj_clear_flag(busIconContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(busIconContainer, 24, 21);
    lv_obj_set_style_bg_color(busIconContainer, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_bg_opa(busIconContainer, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(busIconContainer, 4, 0);

    lv_obj_t *stopNameLabel = lv_label_create(stopHeaderRow);
    lv_obj_set_style_text_font(stopNameLabel, &lv_font_montserrat_14, 0);
    lv_label_set_text(stopNameLabel, STOP_NAME);

    // populated/torn down every render -- see renderArrivals()
    arrivalRowsContainer = lv_obj_create(arrivalsGroup);
    lv_obj_remove_style_all(arrivalRowsContainer);
    lv_obj_clear_flag(arrivalRowsContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(arrivalRowsContainer, lv_pct(100));
    lv_obj_set_height(arrivalRowsContainer, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(arrivalRowsContainer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(arrivalRowsContainer, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(arrivalRowsContainer, 8, 0);

    statusLabel = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(statusLabel, &lv_font_montserrat_8, 0);
    lv_obj_align(statusLabel, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_label_set_text(statusLabel, "connecting...");
}

void displayLoop() {
    static unsigned long lastTick = millis(); // note to self: this is only initialized once :)
    const unsigned long now = millis();
    lv_tick_inc(now - lastTick);
    lastTick = now;

    lv_timer_handler();
    updateStatusLabel();
    updateClockLabel();
    updateDateLabel();
    updateWeatherLabel();
    renderArrivals();
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

void showWeather(const Weather &weather) {
    lastWeather = weather;
    hasWeather = true;
    updateWeatherLabel();
}
