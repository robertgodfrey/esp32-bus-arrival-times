#include "log.h"
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

// public sans for letters, inter for numbers
LV_FONT_DECLARE(inter_medium_14); // high/low temps
LV_FONT_DECLARE(inter_medium_30); // temp & mins (route arrivals)
LV_FONT_DECLARE(inter_medium_46); // time
LV_FONT_DECLARE(public_sans_14); // date, stop
LV_FONT_DECLARE(public_sans_10); // "(sched.)" suffix (route arrivals)
LV_FONT_DECLARE(public_sans_medium_16); // route badges
LV_FONT_DECLARE(public_sans_medium_26); // "min" (route arrivals)
LV_FONT_DECLARE(public_sans_medium_28); // "NOW" (route arrivals)

LV_FONT_DECLARE(icons_weather);
LV_FONT_DECLARE(icon_bus);

static lv_obj_t *clockLabel;
static lv_obj_t *dateLabel;
static lv_obj_t *weatherIconLabel;
static lv_obj_t *currentTempLabel;
static lv_obj_t *highLabel;
static lv_obj_t *lowLabel;
static lv_obj_t *arrivalRowsContainer;
static lv_obj_t *statusLabel;

static bool hasWeather = false;
static Weather lastWeather;
static std::vector<Arrival> lastArrivals;
static int64_t lastServerTimeMs = 0; // API's server time, at the moment of the last successful fetch
static unsigned long deviceMillisAtLastFetch = 0; // millis() at that same moment, used to extrapolate forward
static unsigned long lastSuccessMs = 0; // 0 = never succeeded yet

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

// erikflowers/weather-icons (SIL OFL)
static const char *weatherIconGlyph(const WeatherCondition condition) {
    switch (condition) {
        case WeatherCondition::Clear: return "\uF00D"; // wi-day-sunny
        case WeatherCondition::MainlyClear: return "\uF00C"; // wi-day-sunny-overcast
        case WeatherCondition::PartlyCloudy: return "\uF002"; // wi-day-cloudy
        case WeatherCondition::Overcast: return "\uF013"; // wi-cloudy
        case WeatherCondition::Fog: return "\uF014"; // wi-fog
        case WeatherCondition::Drizzle: return "\uF01C"; // wi-sprinkle
        case WeatherCondition::Rain: return "\uF019"; // wi-rain
        case WeatherCondition::RainShowers: return "\uF01A"; // wi-showers
        case WeatherCondition::Snow: return "\uF01B"; // wi-snow
        case WeatherCondition::Thunderstorm: return "\uF01E"; // wi-thunderstorm
    }
    return "";
}

static void updateWeatherLabel() {
    if (!hasWeather) {
        return;
    }
    char currentText[8];
    snprintf(currentText, sizeof(currentText), "%.0f°", lastWeather.currentTempF);
    lv_label_set_text(currentTempLabel, currentText);

    char highText[8];
    snprintf(highText, sizeof(highText), "%.0f°", lastWeather.highTempF);
    lv_label_set_text(highLabel, highText);

    char lowText[8];
    snprintf(lowText, sizeof(lowText), "%.0f°", lastWeather.lowTempF);
    lv_label_set_text(lowLabel, lowText);
}

static lv_obj_t *addRouteNameBadge(lv_obj_t *parent, const char *text, const lv_color_t bgColor) {
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
    lv_obj_set_style_text_font(label, &public_sans_medium_16, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_text(label, text);
    return badge;
}

static lv_obj_t *addArrivalTimeCell(lv_obj_t *parent, const int64_t minutesAway, const bool scheduled) {
    lv_obj_t *cell = lv_obj_create(parent);
    lv_obj_remove_style_all(cell);
    lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(cell, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cell, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cell, 2, 0);

    lv_obj_t *mainRow = lv_obj_create(cell);
    lv_obj_remove_style_all(mainRow);
    lv_obj_clear_flag(mainRow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(mainRow, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(mainRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(mainRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
    lv_obj_set_style_pad_column(mainRow, 4, 0);

    if (minutesAway == 0) {
        // single label
        lv_obj_t *nowLabel = lv_label_create(mainRow);
        lv_obj_set_style_text_font(nowLabel, &public_sans_medium_28, 0);
        lv_obj_set_style_text_color(nowLabel, lv_color_black(), 0);
        lv_label_set_text(nowLabel, "NOW");
    } else {
        // two labels side by side (number is inter and letters are public sans)
        lv_obj_t *numberLabel = lv_label_create(mainRow);
        lv_obj_set_style_text_font(numberLabel, &inter_medium_30, 0);
        lv_obj_set_style_text_color(numberLabel, lv_color_black(), 0);
        lv_label_set_text(numberLabel, String(minutesAway).c_str());

        lv_obj_t *unitLabel = lv_label_create(mainRow);
        lv_obj_set_style_text_font(unitLabel, &public_sans_medium_26, 0);
        lv_obj_set_style_text_color(unitLabel, lv_color_black(), 0);
        lv_label_set_text(unitLabel, "min");
    }

    if (scheduled) {
        lv_obj_t *schedLabel = lv_label_create(cell);
        lv_obj_set_style_text_font(schedLabel, &public_sans_10, 0);
        lv_obj_set_style_text_color(schedLabel, lv_color_black(), 0);
        lv_label_set_text(schedLabel, "(sched.)");
    }

    return cell;
}

static void updateStatusLabel() {
    char status[48];
    if (lastSuccessMs == 0) {
        snprintf(status, sizeof(status), "Connecting...");
    } else {
        const unsigned long agoMins = (millis() - lastSuccessMs) / 60000;
        if (agoMins == 0) {
            snprintf(status, sizeof(status), "Fetched less than a minute ago");
        } else {
            snprintf(status, sizeof(status), "Fetched %lu min ago", agoMins);
        }
    }
    lv_label_set_text(statusLabel, status);
}

// recomputes "minutes away" for each stored arrival based on how much time has passed since the last successful fetch
static void renderArrivals() {
    const int64_t estimatedServerTimeMs = lastServerTimeMs + (millis() - deviceMillisAtLastFetch);

    lv_obj_clean(arrivalRowsContainer);

    if (lastArrivals.empty()) {
        lv_obj_t *emptyLabel = lv_label_create(arrivalRowsContainer);
        lv_obj_set_width(emptyLabel, lv_pct(100));
        lv_obj_set_style_text_align(emptyLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(emptyLabel, &public_sans_medium_16, 0);
        lv_label_set_text(emptyLabel, "No arrivals");
        return;
    }

    static constexpr int32_t colDsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    static std::vector<int32_t> rowDsc;
    rowDsc.assign(lastArrivals.size() + 1, LV_GRID_CONTENT);
    rowDsc.back() = LV_GRID_TEMPLATE_LAST;
    lv_obj_set_grid_dsc_array(arrivalRowsContainer, colDsc, rowDsc.data());
    lv_obj_set_grid_align(arrivalRowsContainer, LV_GRID_ALIGN_CENTER, LV_GRID_ALIGN_START);

    for (size_t i = 0; i < lastArrivals.size(); i++) {
        const Arrival &arrival = lastArrivals[i];
        const auto row = static_cast<int32_t>(i);
        const ArrivalStatus status = arrivalStatus(arrival);
        const int64_t minutesAway = (arrival.arrivalTimeMs - estimatedServerTimeMs) / 60000;

        lv_obj_t *routeNameBadge = addRouteNameBadge(arrivalRowsContainer, arrival.routeShortName.c_str(), lv_color_hex(0x9C182F));
        lv_obj_set_grid_cell(routeNameBadge, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, row, 1);

        lv_obj_t *timeCell = addArrivalTimeCell(arrivalRowsContainer, minutesAway, status == ArrivalStatus::Scheduled);
        lv_obj_set_grid_cell(timeCell, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, row, 1);
    }

    checkChimes(lastArrivals, estimatedServerTimeMs);
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
    lv_obj_set_style_text_font(clockLabel, &inter_medium_46, 0);
    lv_obj_set_style_text_align(clockLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(clockLabel, "00:00");

    dateLabel = lv_label_create(timeGroup);
    lv_obj_set_width(dateLabel, lv_pct(100));
    lv_obj_set_style_text_font(dateLabel, &public_sans_14, 0);
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
    lv_obj_set_style_pad_row(weatherGroup, 10, 0);
    lv_obj_align_to(weatherGroup, timeGroup, LV_ALIGN_OUT_BOTTOM_MID, 0, 14); // directly under the time group

    lv_obj_t *weatherSeparator = lv_obj_create(weatherGroup);
    lv_obj_remove_style_all(weatherSeparator);
    lv_obj_clear_flag(weatherSeparator, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(weatherSeparator, lv_pct(80), 2);
    lv_obj_set_style_bg_color(weatherSeparator, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_bg_opa(weatherSeparator, LV_OPA_COVER, 0);

    // icon + current temp + stacked high/low all share one row
    lv_obj_t *currentRow = lv_obj_create(weatherGroup);
    lv_obj_remove_style_all(currentRow);
    lv_obj_clear_flag(currentRow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(currentRow, LV_SIZE_CONTENT, 44);
    lv_obj_set_flex_flow(currentRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(currentRow, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(currentRow, 8, 0);

    weatherIconLabel = lv_label_create(currentRow);
    lv_obj_set_style_text_font(weatherIconLabel, &icons_weather, 0);
    lv_label_set_text(weatherIconLabel, "");

    currentTempLabel = lv_label_create(currentRow);
    lv_obj_set_style_text_font(currentTempLabel, &inter_medium_30, 0);
    lv_label_set_text(currentTempLabel, "0");

    // high on top, short divider, low on the bottom
    lv_obj_t *highLowColumn = lv_obj_create(currentRow);
    lv_obj_remove_style_all(highLowColumn);
    lv_obj_clear_flag(highLowColumn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(highLowColumn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(highLowColumn, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(highLowColumn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(highLowColumn, 2, 0);

    highLabel = lv_label_create(highLowColumn);
    lv_obj_set_style_text_font(highLabel, &inter_medium_14, 0);
    lv_label_set_text(highLabel, "0");

    lv_obj_t *highLowDivider = lv_obj_create(highLowColumn);
    lv_obj_remove_style_all(highLowDivider);
    lv_obj_clear_flag(highLowDivider, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(highLowDivider, 16, 1);
    lv_obj_set_style_bg_color(highLowDivider, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_bg_opa(highLowDivider, LV_OPA_COVER, 0);
    lv_obj_set_style_margin_ver(highLowDivider, 2, 0);

    lowLabel = lv_label_create(highLowColumn);
    lv_obj_set_style_text_font(lowLabel, &inter_medium_14, 0);
    lv_label_set_text(lowLabel, "0");

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

    lv_obj_t *arrivalSeparator = lv_obj_create(arrivalsGroup);
    lv_obj_remove_style_all(arrivalSeparator);
    lv_obj_clear_flag(arrivalSeparator, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(arrivalSeparator, lv_pct(80), 2);
    lv_obj_set_style_bg_color(arrivalSeparator, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_bg_opa(arrivalSeparator, LV_OPA_COVER, 0);

    lv_obj_t *stopHeaderRow = lv_obj_create(arrivalsGroup);
    lv_obj_remove_style_all(stopHeaderRow);
    lv_obj_clear_flag(stopHeaderRow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(stopHeaderRow, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(stopHeaderRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(stopHeaderRow, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(stopHeaderRow, 6, 0);

    lv_obj_t *busIconLabel = lv_label_create(stopHeaderRow);
    lv_obj_set_style_text_font(busIconLabel, &icon_bus, 0);
    lv_label_set_text(busIconLabel, "\uF55E"); // icon_bus's one glyph

    lv_obj_t *stopNameLabel = lv_label_create(stopHeaderRow);
    lv_obj_set_style_text_font(stopNameLabel, &public_sans_14, 0);
    lv_label_set_text(stopNameLabel, STOP_NAME);

    arrivalRowsContainer = lv_obj_create(arrivalsGroup);
    lv_obj_remove_style_all(arrivalRowsContainer);
    lv_obj_clear_flag(arrivalRowsContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(arrivalRowsContainer, lv_pct(100));
    lv_obj_set_height(arrivalRowsContainer, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_row(arrivalRowsContainer, 12, 0);
    lv_obj_set_style_pad_column(arrivalRowsContainer, 10, 0);
    lv_obj_set_style_pad_top(arrivalRowsContainer, 6, 0);
}

void displayLoop() {
    static unsigned long lastTick = millis(); // note to self: this is only initialized once :)
    const unsigned long now = millis();
    lv_tick_inc(now - lastTick);
    lastTick = now;

    lv_timer_handler();
    updateClockLabel();
    updateDateLabel();
    updateWeatherLabel();
    renderArrivals();
}

void showArrivals(const std::vector<Arrival> &arrivals, const int64_t serverTimeMs) {
    lastArrivals = arrivals;
    lastServerTimeMs = serverTimeMs;
    deviceMillisAtLastFetch = millis();
    renderArrivals();
}

void showWeather(const Weather &weather) {
    lastWeather = weather;
    hasWeather = true;
    lv_label_set_text(weatherIconLabel, weatherIconGlyph(weather.condition));
    updateWeatherLabel();
}
