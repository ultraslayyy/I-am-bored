#include <lib/stddef.h>
#include <lib/stdint.h>
#include "math.h"

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

double ldexp(double x, int exp) {
    if (x == 0.0) {
        return x;
    }

    if (exp > 0) {
        while (exp--) {
            x *= 2.0;
        } 
    } else {
        while (exp++) {
            x *= 0.5;
        }
    }

    return x;
}

#define TWO_PI   6.28318530717958647692
#define HALF_PI  1.57079632679489661923

#define CORDIC_ITERATIONS 32
#define CORDIC_K_INV 1.6467602581210654

static const double cordic_atan_table[CORDIC_ITERATIONS] = {
    0.7853981633974483,
    0.4636476090008061,
    0.24497866312686414,
    0.12435499454676144,
    0.06241880999595735,
    0.031239833430268277,
    0.015623728620476831,
    0.007812341060101111,
    0.0039062301319669718,
    0.0019531225164788188,
    0.0009765621895593195,
    0.0004882812111948983,
    0.00024414062014936177,
    0.00012207031189367021,
    0.00006103515617420877,
    0.000030517578115526096,
    0.000015258789061315762,
    0.00000762939453110197,
    0.000003814697265606496,
    0.000001907348632810187,
    0.0000009536743164059603,
    0.000000476837158203088,
    0.00000023841857910155797,
    0.000000119209289550780,
    0.0000000596046447753906,
    0.0000000298023223876953,
    0.0000000149011611938477,
    0.00000000745058059692383,
    0.00000000372529029846191,
    0.00000000186264514923096,
    0.00000000093132257461548,
    0.00000000046566128730774
};

static void cordic_sincos(double angle, double *s, double *c) {
    angle = fmod(angle, 2.0 * M_PI);

    if (angle > M_PI) {
        angle -= TWO_PI;
    } else if (angle < -M_PI) {
        angle += TWO_PI;
    }

    int sign_s = 1;
    int sign_c = 1;

    if (angle > HALF_PI) {
        angle -= M_PI;
        sign_s = -1;
        sign_c = -1;
    } else if (angle < -HALF_PI) {
        angle += M_PI;
        sign_s = -1;
        sign_c = -1;
    }

    double x = CORDIC_K_INV;
    double y = 0.0;
    double z = angle;

    for (size_t i = 0; i < CORDIC_ITERATIONS; ++i) {
        double x_old = x;

        if (z >= 0.0) {
            x -= y * ldexp(1.0, -(int)i);
            y += x_old * ldexp(1.0, -(int)i);
            z -= cordic_atan_table[i];
        } else {
            x += y * ldexp(1.0, -(int)i);
            y -= x_old * ldexp(1.0, -(int)i);
            z += cordic_atan_table[i];
        }
    }

    *c = x * sign_c;
    *s = y * sign_s;
}

double sin(double x) {
    double s, c;
    cordic_sincos(x, &s, &c);
    return s;
}

double cos(double x) {
    double s, c;
    cordic_sincos(x, &s, &c);
    return c;
}

double tan(double x) {
    double s, c;
    cordic_sincos(x, &s, &c);
    return s / c;
}