#include "ac97.h"
#include "pc_speaker.h"

void audio_init(void) {
    ac97_init();
    
    speaker_init();
}