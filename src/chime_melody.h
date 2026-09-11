#pragma once

namespace chime_melody {
    struct Note {
        int hz;
        int ms;
    };

    struct Melody {
        const char *name;
        const Note *notes;
        int noteCount;
    };

    constexpr int NOTE_D4 = 294;
    constexpr int NOTE_E4 = 330;
    constexpr int NOTE_FS4 = 370; // F#4
    constexpr int NOTE_G4 = 392;
    constexpr int NOTE_A4 = 440;
    constexpr int NOTE_B4 = 494;
    constexpr int NOTE_C5 = 523;
    constexpr int NOTE_CS5 = 554; // C#5
    constexpr int NOTE_D5 = 587;
    constexpr int NOTE_E5 = 659;
    constexpr int NOTE_FS5 = 740; // F#5
    constexpr int NOTE_G5 = 784;
    constexpr int NOTE_A5 = 880;

    constexpr Note NOTIFICATION[] = {
        {1200, 100},
        {1600, 300},
    };

    constexpr Note ASCENT[] = {
        {1200, 120},
        {1350, 100},
        {1500, 100},
        {1600, 200},
    };

    constexpr Note NEW_YORK_SUBWAY[] = {
        {NOTE_E5, 1000},
        {NOTE_C5, 1000},
    };

    constexpr Note MONTREAL_SUBWAY[] = {
        {NOTE_FS4, 600},
        {NOTE_B4, 600},
        {NOTE_FS5, 1000},
    };

    constexpr Note SEOUL_SUBWAY[] = {
        {NOTE_D5, 200},
        {NOTE_A4, 200},
        {NOTE_D5, 200},
        {NOTE_E5, 200},
        {NOTE_FS5, 200},
        {NOTE_E5, 200},
        {NOTE_D5, 200},
        {NOTE_A4, 200},

        {NOTE_B4, 200},
        {NOTE_G4, 200},
        {NOTE_B4, 200},
        {NOTE_D5, 200},
        {NOTE_G5, 200},
        {NOTE_FS4, 200},
        {NOTE_E4, 200},
        {NOTE_D4, 200},

        {NOTE_CS5, 200},
        {NOTE_A4, 200},
        {NOTE_CS5, 200},
        {NOTE_E5, 200},
        {NOTE_A5, 200},
        {NOTE_G5, 200},
        {NOTE_FS5, 200},
        {NOTE_E5, 200},

        {NOTE_FS5, 200},
        {NOTE_A5, 200},
        {NOTE_E5, 200},
        {NOTE_A5, 200},
        {NOTE_D5, 800},
    };


    constexpr Melody MELODIES[] = {
        {"Notification", NOTIFICATION, sizeof(NOTIFICATION) / sizeof(NOTIFICATION[0])},
        {"Simple Ascent", ASCENT, sizeof(ASCENT) / sizeof(ASCENT[0])},
        {"NYC Subway", NEW_YORK_SUBWAY, sizeof(NEW_YORK_SUBWAY) / sizeof(NEW_YORK_SUBWAY[0])},
        {"Montreal Subway", MONTREAL_SUBWAY, sizeof(MONTREAL_SUBWAY) / sizeof(MONTREAL_SUBWAY[0])},
        {"Seoul Subway", SEOUL_SUBWAY, sizeof(SEOUL_SUBWAY) / sizeof(SEOUL_SUBWAY[0])},
    };

    // which entry in MELODIES plays
    constexpr int SELECTED_MELODY = 4;
}
