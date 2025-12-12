#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern "C" {

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

int divide(int a, int b) {
    return a / b;
}

double power(double a, double b) {
    return pow(a, b);
}

double hypot_wasm(double a, double b) {
    return hypot(a, b);
}

double PI() {
    return M_PI;
}

double E() {
    return M_E;
}

char* toBase64(const unsigned char* str, size_t length) {
    static const char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    size_t encoded_length = 4 * ((length + 2) / 3);
    char* data = (char*)malloc(encoded_length + 1);
    if (!data) return NULL;

    size_t i = 0, j = 0;
    while (i < length) {
        uint32_t buffer = 0;
        int remaining = (int)(length - i);

        buffer |= ((uint32_t)str[i++]) << 16;
        if (remaining > 1) {
            buffer |= ((uint32_t)str[i++]) << 8;
        }
        if (remaining > 2) {
            buffer |= ((uint32_t)str[i++]);
        }

        data[j++] = base64_alphabet[(buffer >> 18) & 0x3F];
        data[j++] = base64_alphabet[(buffer >> 12) & 0x3F];
        data[j++] = (remaining > 1) ? base64_alphabet[(buffer >> 6) & 0x3F] : '=';
        data[j++] = (remaining > 2) ? base64_alphabet[buffer & 0x3F] : '=';
    }
    data[encoded_length] = '\0';
    return data;
}

}