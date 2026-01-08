#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
    #define OS_SLEEP(seconds) Sleep((seconds) * 1000)
#else
    #include <unistd.h>
    #define OS_SLEEP(seconds) sleep(seconds);
#endif

const int MAX_ALLOCATION_GB = 6;
const size_t CHUNK_SIZE_MB = 256;
const size_t CHUNK_SIZE = 268435456ULL;

int main() {
    volatile char* ptr;
    size_t total_allocated = 0;
    int chunk_count = 0;

    while (1) {
        ptr = (volatile char*)malloc(CHUNK_SIZE);

        if (ptr == NULL) {
            break;
        }

        for (size_t i = 0; i < CHUNK_SIZE; i += 4096) {
            ptr[i] = (char)(chunk_count % 256);
        }

        total_allocated += CHUNK_SIZE;
        chunk_count++;
        double total_mb = (double)total_allocated / (1024 * 1024);
        double total_gb = total_mb / 1024.0;

        if (total_gb > MAX_ALLOCATION_GB) {
            break;
        }
    }

    while (1) {
        OS_SLEEP(10);
    }

    return 0;
}