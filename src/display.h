#pragma once

#include <Arduino.h>
#include <vector>
#include "oba_client.h"
#include "weather_client.h"

void displayInit();

// call every loop() iteration -- handles LVGL tick/render, the status label,
// and re-rendering arrival countdowns as time passes
void displayLoop();

// currentTimeMs is the API's reported server time at fetch -- used as an
// anchor so displayLoop() can keep computing accurate countdowns afterward
void showArrivals(const std::vector<Arrival> &arrivals, int64_t currentTimeMs);
void recordFetchFailed();

void showWeather(const Weather &weather);
