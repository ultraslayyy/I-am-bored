#include <string.h>
#include <stdlib.h>
#include "helpers.h"

void swap_bytes(void *a, void *b, size_t size) {
    if (a == b) return;

    unsigned char tmp[256];
    if (size <= sizeof(tmp)) {
        memcpy(tmp, a, size);
        memcpy(a, b, size);
        memcpy(b, tmp, size);
    } else {
        unsigned char *buf = malloc(size);
        memcpy(buf, a, size);
        memcpy(a, b, size);
        memcpy(b, buf, size);
        free(buf);
    }
}

size_t min_size(size_t a, size_t b) {
    return a < b ? a : b;
}