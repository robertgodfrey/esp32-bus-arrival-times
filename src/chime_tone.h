#pragma once

// Configures the LEDC/PWM channel used to drive the onboard amp -- call once from setup().
void chimeToneSetup();

// Plays the selected melody (see chime_melody.h) on a background task and returns immediately --
// some melodies run several seconds long, so this doesn't block the caller while it plays. Safe
// to call again before the current chime finishes; it's just ignored while one is already playing.
void playChimeTone();
