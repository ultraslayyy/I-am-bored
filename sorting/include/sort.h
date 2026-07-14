#pragma once
#include <stddef.h>

typedef int (*cmp_fn)(const void *, const void *);

void insertion_sort(void *base, size_t n, size_t size, cmp_fn cmp);
void quick_sort    (void *base, size_t n, size_t size, cmp_fn cmp);
void sleep_sort    (int  *arr, size_t n);
void bubble_sort   (void *base, size_t n, size_t size, cmp_fn cmp);
void merge_sort(void *base, size_t n, size_t size, cmp_fn cmp);