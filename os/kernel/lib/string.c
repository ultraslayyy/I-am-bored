#include "string.h"

size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) ++len;
    return len;
}

int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) {
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if (a[i] != b[i] || a[i] == 0 || b[i] == 0) 
            return (unsigned char)a[i] - (unsigned char)b[i];
    }
    return 0;
}

char *strchr(const char *s, char c) {
    while (*s) {
        if (*s == c) return (char *)s;
        s++;
    }
    return 0;
}

void strcpy(char *dst, const char *src) {
    while ((*dst++ = *src++));
}
