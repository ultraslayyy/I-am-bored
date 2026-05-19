struct AudioTrack {
    int id;
    int sample_count;
    int *samples;
};

int master_multiplier = 2;

void apply_master_mix(struct AudioTrack *tracks, int track_count) {
    int multiplier = master_multiplier;

    for (int i = 0; i < track_count; ++i) {
        int *current_samples = tracks[i].samples;
        int count = tracks[i].sample_count;

        for (int j = 0; j < count; ++j) {
            current_samples[j] *= multiplier;
        }
    }
}