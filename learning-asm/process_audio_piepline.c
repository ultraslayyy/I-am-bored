#include <stddef.h>

// 1. Structural Layout (Structs)
struct ChannelConfig {
    int channel_id;       // 4 bytes | Offset +0
    short volume_target;  // 2 bytes | Offset +4
    char active;          // 1 byte  | Offset +6
    char padding;         // 1 byte  | Offset +7 (alignment)
    const char* tag;      // 8 bytes | Offset +8 (Pointer to string)
};

// 2. Global Initialized Data (.data)
int global_multiplier = 3;
const char* status_success = "PASS";
const char* status_fail    = "FAIL";

// 3. Global Uninitialized Data (.bss)
int dynamic_audio_buffer[256];

// 4. The Master Function
int process_audio_pipeline(struct ChannelConfig* channels, int channel_count) {
    // Local stack variables
    int total_processed_samples = 0;
    int processing_error = 0;

    // Outer Loop: Stride through Struct Array
    for (int i = 0; i < channel_count; i++) {
        // Zero-extend byte/short fields (movzx practice)
        if (!channels[i].active) {
            continue;
        }

        int target = channels[i].volume_target;
        const char* current_tag = channels[i].tag;

        // String check: Check if tag is valid (First char isn't null)
        if (current_tag == NULL || current_tag[0] == '\0') {
            processing_error = 1;
            break;
        }

        // Inner Loop: Process the global .bss buffer based on target volume
        // We simulate filling and scaling 10 samples per active channel
        for (int j = 0; j < 10; j++) {
            int buffer_index = total_processed_samples;
            
            // Raw math mix: (j * global_multiplier) + target
            int raw_sample = (j * global_multiplier) + target;

            // Math Division: Safe Headroom calculation (idiv practice)
            // Let's say we must divide the sample by an arbitrary headroom factor of 2
            int finalized_sample = raw_sample / 2;

            dynamic_audio_buffer[buffer_index] = finalized_sample;
            total_processed_samples++;
        }
    }

    // Stack Cleanup & Ternary Return
    if (processing_error != 0) {
        return -1;
    }
    return total_processed_samples;
}