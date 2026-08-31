#pragma once

// Configures the PWM channel used to drive the onboard amp -- call once from setup().
void chimeToneSetup();

// Plays a short attention tone. Blocks briefly (a few hundred ms) while it plays.
void playChimeTone();
