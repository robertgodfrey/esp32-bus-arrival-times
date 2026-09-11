#include <Arduino.h>
#include "chime_melody.h"
#include "chime_tone.h"
#include "config.h"
#include "log.h"

static constexpr uint8_t CHIME_LEDC_CHANNEL = 0;
static constexpr uint8_t CHIME_LEDC_RESOLUTION_BITS = 10;
static constexpr uint32_t CHIME_LEDC_MAX_DUTY = (1 << CHIME_LEDC_RESOLUTION_BITS) - 1;
static constexpr float CHIME_VOLUME = 0.002f; // 0.5 max

static constexpr int CHIME_FADE_STEPS = 8;
static constexpr int CHIME_FADE_STEP_MS = 2; // ~16ms fade in, ~16ms fade out
static constexpr int CHIME_FADE_MS = CHIME_FADE_STEPS * CHIME_FADE_STEP_MS;

static void rampDuty(const uint32_t targetDuty, const bool rampUp) {
    for (int step = 0; step <= CHIME_FADE_STEPS; step++) {
        const int effectiveStep = rampUp ? step : CHIME_FADE_STEPS - step;
        ledcWrite(CHIME_LEDC_CHANNEL, targetDuty * effectiveStep / CHIME_FADE_STEPS);
        delay(CHIME_FADE_STEP_MS);
    }
}

static TaskHandle_t chimeTaskHandle = nullptr;

static void chimeTask(void *) {
    constexpr auto targetDuty = static_cast<uint32_t>(CHIME_VOLUME * CHIME_LEDC_MAX_DUTY);
    const chime_melody::Melody &melody = chime_melody::MELODIES[chime_melody::SELECTED_MELODY];

    for (int i = 0; i < melody.noteCount; i++) {
        const chime_melody::Note &note = melody.notes[i];
        ledcWriteTone(CHIME_LEDC_CHANNEL, note.hz); // resets duty to 50% -- immediately overridden below

        const bool isFirst = i == 0;
        const bool isLast = i == melody.noteCount - 1;

        if (isFirst) {
            rampDuty(targetDuty, true);
            delay(note.ms > CHIME_FADE_MS ? note.ms - CHIME_FADE_MS : 0);
        } else if (isLast) {
            ledcWrite(CHIME_LEDC_CHANNEL, targetDuty);
            delay(note.ms > CHIME_FADE_MS ? note.ms - CHIME_FADE_MS : 0);
            rampDuty(targetDuty, false);
        } else {
            ledcWrite(CHIME_LEDC_CHANNEL, targetDuty);
            delay(note.ms);
        }
    }

    ledcWriteTone(CHIME_LEDC_CHANNEL, 0); // silence

    chimeTaskHandle = nullptr;
    vTaskDelete(nullptr);
}

void chimeToneSetup() {
    ledcSetup(CHIME_LEDC_CHANNEL, 2000, CHIME_LEDC_RESOLUTION_BITS);
    ledcAttachPin(CHIME_GPIO, CHIME_LEDC_CHANNEL);
}

void playChimeTone() {
    LOGF("playChimeTone");
    if (chimeTaskHandle != nullptr) {
        LOGF("chime already playing, skipping");
        return;
    }
    xTaskCreate(chimeTask, "chimeTask", 2048, nullptr, 1, &chimeTaskHandle);
}
