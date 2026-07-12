#include <cpuid.h>
#include <stdint.h> 

extern void matmul_avx512_fp16(float *a, float *b, float *c, int n);
extern void matmul_scalar     (float *a, float *b, float *c, int n);
extern void matmul_sse        (float *a, float *b, float *c, int n);

#if defined(_MSC_VER)
#include <intrin.h>

static void cpuidex(unsigned int leaf, unsigned int subleaf,
                    unsigned int *a, unsigned int *b,
                    unsigned int *c, unsigned int *d) {
    int r[4];
    __cpuidex(r, leaf, subleaf);
    *a = r[0];
    *b = r[1];
    *c = r[2];
    *d = r[3];
}

static uint64_t xgetbv(unsigned int index) {
    return _xgetbv(index);
}

#else
#include <immintrin.h>

static void cpuidex(unsigned int leaf, unsigned int subleaf,
                    unsigned int *a, unsigned int *b,
                    unsigned int *c, unsigned int *d) {
    __cpuid_count(leaf, subleaf, *a, *b, *c, *d);
}

static uint64_t xgetbv(unsigned int index) {
    return _xgetbv(index);
}

#endif

static int has_avx512_fp16(void) {
    unsigned int eax, ebx, ecx, edx;

    /**
     * Check CPUID leaf 1:
     * - OSXSAVE
     * - AVX
     */
    cpuidex(1, 0, &eax, &ebx, &ecx, &edx);

    if (!(ecx & (1u << 27))) { // OSXSAVE
        return 0;
    }

    if (!(ecx & (1u << 28))) { // AVX
        return 0;
    }

    /**
     * Check that OS enabled:
     * XMM (bit 1)
     * YMM (bit 2)
     * Opmask (bit 5)
     * ZMM_Hi256 (bit 6)
     * Hi16_ZMM (bit 7)
     */
    uint64_t xcr0 = xgetbv(0);

    const uint64_t required_xcr0 =
        (1ull << 1) |  // XMM
        (1ull << 2) |  // YMM
        (1ull << 5) |  // opmask
        (1ull << 6) |  // ZMM_Hi256
        (1ull << 7);   // Hi16_ZMM

    if ((xcr0 & required_xcr0) != required_xcr0)
        return 0;


    /**
     * Check CPUID leaf 7 subleaf 0:
     * EDX bit 23 = AVX512-FP16
     */
    cpuidex(7, 0, &eax, &ebx, &ecx, &edx);

    if (!(edx & (1u << 23))) {
        return 0;
    }

    return 1;
}

int has_sse(void) {
    unsigned int eax, ebx, ecx, edx;

    if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        return 0;
    }

    return (edx & (1u << 25)) != 0;
}

int main() {
    float a[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float b[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float c[] = {};
    int   n   = 8;

    if (has_sse()) {
        matmul_sse(a, b, c, n);
    }
}