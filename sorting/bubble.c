#include "helpers.h"
#include "sort.h"

void bubble_sort(void *base, size_t n, size_t size, cmp_fn cmp) {
    unsigned char *arr = base;

    for (size_t i = 0; i < n - 1; ++i) {
        int swapped = 0;

        for (size_t j = 0; j < n - i - 1; ++j) {
            unsigned char *a = arr + j * size;
            unsigned char *b = arr + (j + 1) * size;

            if (cmp(a, b) > 0) {
                swap_bytes(a, b, size);
                swapped = 1;
            }
        }

        if (!swapped) break;
    }
}