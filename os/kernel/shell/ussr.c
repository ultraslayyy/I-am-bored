#include <drivers/audio/pc_speaker.h>
#include <timer/timer.h>
#include <arch.h>

typedef struct {
    uint16_t frequency;
    uint16_t duration_ms;
} melody_note_t;

// Note frequencies
#define NOTE_EB4 311
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_AB4 415
#define NOTE_BB4 466

#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_DB5 554
#define NOTE_EB5 622
#define NOTE_F5  698
#define NOTE_G5  784

#define REST 0

// Note lengths
#define HALF           1818
#define DOTTED_QUARTER 1364
#define QUARTER        909
#define DOTTED_EIGHTH  682
#define EIGHTH         455
#define SIXTEENTH      227

static const melody_note_t ussr_melody[] = {
    // Measure 1
    { NOTE_EB5,  HALF + QUARTER }, // Half tied to quarter
    { REST,      EIGHTH },
    { NOTE_BB4,  EIGHTH },

    // Measure 2
    { NOTE_EB5,  QUARTER },
    { NOTE_BB4,  DOTTED_EIGHTH },
    { NOTE_C5,   SIXTEENTH },
    { NOTE_D5,   QUARTER },
    { NOTE_G4,   EIGHTH },
    { NOTE_G4,   EIGHTH },

    // Measure 3
    { NOTE_C5,   QUARTER },
    { NOTE_BB4,  DOTTED_EIGHTH },
    { NOTE_AB4,  SIXTEENTH },
    { NOTE_BB4,  QUARTER },
    { NOTE_EB4,  EIGHTH },
    { NOTE_EB4,  EIGHTH },

    // Measure 4
    { NOTE_F4,   QUARTER },
    { NOTE_F4,   DOTTED_EIGHTH },
    { NOTE_G4,   SIXTEENTH },
    { NOTE_AB4,  QUARTER },
    { NOTE_AB4,  DOTTED_EIGHTH },
    { NOTE_BB4,  SIXTEENTH },

    // Measure 5
    { NOTE_C5,   QUARTER },
    { NOTE_D5,   EIGHTH },
    { NOTE_EB5,  EIGHTH },
    { NOTE_F5,  DOTTED_QUARTER },
    { NOTE_BB4,  EIGHTH },

    // Measure 6
    { NOTE_G5,   QUARTER },
    { NOTE_F5,   DOTTED_EIGHTH },
    { NOTE_EB5,  SIXTEENTH },
    { NOTE_F5,   QUARTER },
    { NOTE_D5,   EIGHTH },
    { NOTE_BB4,  EIGHTH },

    // Measure 7
    { NOTE_EB5, QUARTER },
    { NOTE_D5,  DOTTED_EIGHTH },
    { NOTE_C5,  SIXTEENTH },
    { NOTE_D5,  QUARTER },
    { NOTE_G4,  EIGHTH },
    { NOTE_G4,  EIGHTH },

    // Measure 8
    { NOTE_C5,  QUARTER },
    { NOTE_BB4, DOTTED_EIGHTH },
    { NOTE_AB4, SIXTEENTH },
    { NOTE_BB4, QUARTER },
    { NOTE_G4,  DOTTED_EIGHTH },
    { NOTE_G4,  SIXTEENTH },

    // Measure 9
    { NOTE_EB5, QUARTER },
    { NOTE_D5,  DOTTED_EIGHTH },
    { NOTE_C5,  SIXTEENTH },
    { NOTE_BB4, DOTTED_QUARTER },
    { REST,     EIGHTH },

    // Measure 10
    { NOTE_G5,  HALF },
    { NOTE_F5,  EIGHTH },
    { NOTE_EB5, EIGHTH },
    { NOTE_D5,  EIGHTH },
    { NOTE_EB5, EIGHTH },

    // Measure 11
    { NOTE_F5,  DOTTED_EIGHTH },
    { NOTE_BB4, EIGHTH },
    { NOTE_BB4, DOTTED_QUARTER },
    { REST,     EIGHTH },

    // Measure 12
    { NOTE_F5,  HALF },
    { NOTE_D5,  EIGHTH },
    { NOTE_C5,  EIGHTH },
    { NOTE_BB4, EIGHTH },
    { NOTE_C5,  EIGHTH }
};

int cmd_ussr(int argc, char **argv) {
    (void)argc;
    (void)argv;

    task_set_cleanup(speaker_stop);

    uint32_t count = sizeof(ussr_melody) / sizeof(ussr_melody[0]);

    for (uint32_t i = 0; i < count; ++i) {
        if (ussr_melody[i].frequency == REST) {
            speaker_stop();
        } else {
            speaker_play(ussr_melody[i].frequency);
        }

        delay_ms(ussr_melody[i].duration_ms); 
    }

    return 0;
}