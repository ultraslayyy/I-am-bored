#include <math.h>
#include "helpers.h"
#include "sort.h"

#define INSERTION_THRESHOLD 16

static size_t partition(
    unsigned char *arr,
    size_t lo,
    size_t hi,
    size_t size,
    cmp_fn cmp
) {
    size_t mid = lo + (hi - lo) / 2;

    if (cmp(arr + lo * size, arr + mid * size) > 0) {
        swap_bytes(arr + lo * size, arr + mid * size, size);
    }
    if (cmp(arr + lo * size, arr + hi * size) > 0) {
        swap_bytes(arr + lo * size, arr + hi * size, size);
    }
    if (cmp(arr + mid * size, arr + hi * size) > 0) {
        swap_bytes(arr + mid * size, arr + hi * size, size);
    }

    swap_bytes(arr + mid * size, arr + hi * size, size);

    size_t i = lo;
    for (size_t j = lo; i < hi; ++j) {
        if (cmp(arr + j * size, arr + hi * size) < 0) {
            swap_bytes(arr + i * size, arr + j * size, size);
            i++;
        }
    }

    swap_bytes(arr + i * size, arr + hi * size, size);
    return i;
}

static void quick_sort_impl(
    unsigned char *arr,
    size_t lo,
    size_t hi,
    size_t size,
    cmp_fn cmp
) {
    while (lo < hi) {
        if (hi - lo < INSERTION_THRESHOLD) {
            insertion_sort(arr + lo * size, hi - lo + 1, size, cmp);
            return;
        }

        size_t p = partition(arr, lo, hi, size, cmp);

        if (p - lo < hi - p) {
            quick_sort_impl(arr, lo, p - 1, size, cmp);
            lo = p + 1;
        } else {
            quick_sort_impl(arr, p + 1, hi, size, cmp);
            hi = p - 1;
        }
    }
}

void quick_sort(void *base, size_t n, size_t size, cmp_fn cmp) {
    if (n < 2) return;
    quick_sort_impl(base, 0, n - 1, size, cmp);
}