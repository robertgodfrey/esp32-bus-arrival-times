#include <Arduino.h>
#include "chime_tone.h"
#include "config.h"

// channel 0 -- esp32_smartdisplay claims the *last* LEDC channel for the backlight
// (see PWM_CHANNEL_BCKL in its source), so the low end is safe to use here
static constexpr uint8_t CHIME_LEDC_CHANNEL = 0;
static constexpr uint8_t CHIME_LEDC_RESOLUTION_BITS = 10;

void chimeToneSetup() {
    ledcSetup(CHIME_LEDC_CHANNEL, 2000, CHIME_LEDC_RESOLUTION_BITS);
    ledcAttachPin(CHIME_GPIO, CHIME_LEDC_CHANNEL);
}

void playChimeTone() {
    ledcWriteTone(CHIME_LEDC_CHANNEL, 1600);
    delay(150);
    ledcWriteTone(CHIME_LEDC_CHANNEL, 1200);
    delay(150);
    ledcWriteTone(CHIME_LEDC_CHANNEL, 0); // silence
}
