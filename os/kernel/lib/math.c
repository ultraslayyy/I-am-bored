#include <lib/stddef.h>
#include <lib/stdint.h>

double fmod(double x, double y) {
    // Handle edge cases
    if (y == 0.0 || x != x || y != y) {
        return (0.0 / 0.0); // Return NaN
    }

    double abs_x = (x < 0.0) ? -x : x;
    double abs_y = (y < 0.0) ? -y : y;

    if (abs_x < abs_y) {
        return x;
    }

    if (abs_x == abs_y) {
        return (x < 0.0) ? -0.0 : 0.0;
    }

    double scaled_y = abs_y;
    while (scaled_y <= abs_x / 2.0) {
        scaled_y *= 2.0;
    }

    while (scaled_y >= abs_y) {
        if (abs_x >= scaled_y) {
            abs_x -= scaled_y;
        }
        scaled_y /= 2.0;
    }

    return (x < 0.0) ? -abs_x : abs_x;
}

double sqrt(double x) {
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

double cbrt(double x) {
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

// TODO: double exponent
double pow(double base, unsigned int exponent) {
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

double fmin(double a, double b) {
    if (a > b) {
        return b;
    } else {
        return a;
    }
}

double fmax(double a, double b) {
    if (a < b) {
        return b;
    } else {
        return a;
    }
}