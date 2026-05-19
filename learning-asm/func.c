int get_gain() {
    return 2;
}

int apply_gain(int sample, int gain) {
    return sample * gain;
}

int process_audio(int input_sample) {
    int gain = get_gain();
    int result = apply_gain(input_sample, gain);
    return result;
}