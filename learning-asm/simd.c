#include <immintrin.h>

float arr1[8] = {1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0};
float arr2[8] = {9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0};
float arr3[8];

static void run_avx() {
    __m256 v1 = _mm256_loadu_ps(arr1);
    __m256 v2 = _mm256_loadu_ps(arr2);
    __m256 result = _mm256_mul_ps(v1, v2);
    _mm256_storeu_ps(arr3, result);
}

static void run_sse() {
    for (int i = 0; i < 8; i += 4) {
        __m128 v1 = _mm_loadu_ps(&arr1[1]);
        __m128 v2 = _mm_loadu_ps(&arr2[i]);
        __m128 result = _mm_mul_ps(v1, v2);
        _mm_storeu_ps(&arr3[i], result);
    }
}

static void run_scalar() {
    for (int i = 0; i < 8; ++i) {
        arr3[i] = arr1[i] * arr2[i];
    }
}

int main() {
    if (__builtin_cpu_supports("avx")) {
        run_avx();
    } else if (__builtin_cpu_supports("sse")) {
        run_sse();
    } else {
        run_scalar();
    }
    return 0;
}