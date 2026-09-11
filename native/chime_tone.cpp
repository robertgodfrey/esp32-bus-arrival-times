#include <SDL2/SDL.h>
#include <cmath>
#include <vector>
#include "chime_melody.h"
#include "chime_tone.h"

// approximates what ledcWriteTone() drives directly in hardware on the ESP32 side
namespace {
    constexpr int SAMPLE_RATE_HZ = 44100;
    constexpr float VOLUME = 0.25f;
    constexpr float PI_F = 3.14159265358979323846f;
    constexpr int FADE_MS = 16;

    SDL_AudioDeviceID audioDevice = 0;
}

void chimeToneSetup() {
    if (SDL_WasInit(SDL_INIT_AUDIO) == 0 && SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        SDL_Log("chime: SDL audio init failed: %s", SDL_GetError());
        return;
    }

    SDL_AudioSpec want{};
    want.freq = SAMPLE_RATE_HZ;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 512;

    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, 0);
    if (audioDevice == 0) {
        SDL_Log("chime: SDL_OpenAudioDevice failed: %s", SDL_GetError());
        return;
    }
    SDL_PauseAudioDevice(audioDevice, 0);
}

void playChimeTone() {
    if (audioDevice == 0 || SDL_GetQueuedAudioSize(audioDevice) > 0) {
        return; // not set up/already playing
    }

    const chime_melody::Melody &melody = chime_melody::MELODIES[chime_melody::SELECTED_MELODY];
    int totalMs = 0;
    for (int i = 0; i < melody.noteCount; i++) {
        totalMs += melody.notes[i].ms;
    }
    const int totalSamples = totalMs * SAMPLE_RATE_HZ / 1000;
    const int fadeSamples = FADE_MS * SAMPLE_RATE_HZ / 1000;

    std::vector<int16_t> samples;
    samples.reserve(totalSamples);

    int noteIndex = 0;
    int noteStartSample = 0;
    int noteEndSample = melody.notes[0].ms * SAMPLE_RATE_HZ / 1000;

    for (int i = 0; i < totalSamples; i++) {
        while (i >= noteEndSample && noteIndex < melody.noteCount - 1) {
            noteIndex++;
            noteStartSample = noteEndSample;
            noteEndSample += melody.notes[noteIndex].ms * SAMPLE_RATE_HZ / 1000;
        }

        float envelope = 1.0f;
        if (i < fadeSamples) {
            envelope = static_cast<float>(i) / fadeSamples;
        } else if (i > totalSamples - fadeSamples) {
            envelope = static_cast<float>(totalSamples - i) / fadeSamples;
        }

        const float t = static_cast<float>(i - noteStartSample) / SAMPLE_RATE_HZ;
        const float square = sinf(2.0f * PI_F * melody.notes[noteIndex].hz * t) >= 0.0f ? 1.0f : -1.0f;
        samples.push_back(static_cast<int16_t>(square * envelope * VOLUME * 32767.0f));
    }

    SDL_QueueAudio(audioDevice, samples.data(), static_cast<Uint32>(samples.size() * sizeof(int16_t)));
}
