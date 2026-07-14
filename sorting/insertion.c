#include <string.h>
#include "sort.h"

void insertion_sort(void *base, size_t n, size_t size, cmp_fn cmp) {
    unsigned char *arr = base;

    for (size_t i = 1; i < n; ++i) {
        unsigned char tmp = malloc(size);
        memcpy(tmp, arr + i * size, size);

        size_t j = i;
        while (j > 0 && cmp(arr + (j - 1) * size, tmp) > 0) {
            memmove(arr + j * size, arr + (j - 1) * size, size);
            j--;
        }

        memcpy(arr + j * size, tmp, size);
    }
}