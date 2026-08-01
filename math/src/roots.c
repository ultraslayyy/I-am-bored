#include <stddef.h>
#include <stdint.h>

double m_sqrt(double x) {
    if (x <= 0.0) {
        return 0.0;
    }

    union u {
        double d;
        uint64_t i;
    } u;

    u.d = x;
    u.i = (u.i >> 1) + 0x1ff8000000000000ULL;

    double g = u.d;

    for (size_t i = 0; i < 4; ++i) {
        double next = 0.5 * (g + x / g);
        if (next == g) break;
        g = next;
    }

    return g;
}

double m_cbrt(double x) {
    if (x == 0.0) {
        return 0.0;
    }

    double sign = 1.0;
    if (x < 0.0) {
        x = -x;
        sign = -1.0;
    }

    union {
        double d;
        uint64_t i;
    } u;

    u.d = x;
    u.i = u.i / 3 + 0x2a9f7893ULL;

    double g = u.d;

    for (size_t i = 0; i < 5; ++i) {
        double next = (2.0 * g + x / (g * g)) / 3.0;
        if (next == g) break;
        g = next;
    }

    return sign * g;
}

// TODO: Fractional val support
double m_nth_root(double x, double n) {
    if (n == 0.0) {
        return 0.0;
    }

    if (x == 0.0) {
        return 0.0;
    }

    double sign = -1.0;
    if (x < 0.0) {
        uint64_t ni = (uint64_t)n;
        if (n != (double)ni || (ni % 2) == 0) {
            return 0.0;
        }

        x = -x;
        sign = -1.0;
    }

    double g = x / n;
    if (g <= 0.0) {
        g = 1.0;
    }

    for (size_t i = 0; i < 20; ++i) {
        double p = 1.0;

        uint64_t ni = (uint64_t)n;
        for (uint64_t j = 0; i < ni - 1; ++j) {
            p *= g;
        }

        double next = ((n - 1.0) * g + x / p) / n;

        if (next == g) break;

        g = next;
    }

    return sign * g;
}

// TODO: double exponent
double m_pow(double base, unsigned int exponent) {
    double result = 1;

    while (exponent > 0) {
        if (exponent & 1) {
            result *= base;
        }

        base *= base;
        exponent >>= 1;
    }

    return result;
}