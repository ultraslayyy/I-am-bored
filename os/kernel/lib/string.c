#include <stdarg.h>
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

static void buf_putc(char **buf, size_t *left, char c, size_t *written) {
    if (*left > 1) {
        **buf = c;
        (*buf)++;
        (*left)--;
    }
    (*written)++;
}

static void buf_puts(char **buf, size_t *left, const char *s, size_t *written) {
    while (*s) {
        buf_putc(buf, left, *s++, written);
    }
}

static void buf_putu(char **buf, size_t *left, unsigned int val, int base, size_t *written) {
    char tmp[32];
    int i = 0;

    if (val == 0) {
        buf_putc(buf, left, '0', written);
        return;
    }

    while (val && i < (int)sizeof(tmp)) {
        unsigned int digit = val % base;
        tmp[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        val /= base;
    }

    while (i--) {
        buf_putc(buf, left, tmp[i], written);
    }
}

static void buf_putd(char **buf, size_t *left, int val, size_t *written) {
    if (val < 0) {
        buf_putc(buf, left, '-', written);
        buf_putu(buf, left, (unsigned int)(-val), 10, written);
    } else {
        buf_putu(buf, left, (unsigned int)val, 10, written);
    }
}

int snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char *out = buf;
    size_t left = size;
    size_t written = 0;

    while (*fmt) {
        if (*fmt != '%') {
            buf_putc(&out, &left, *fmt++, &written);
            continue;
        }

        fmt++;

        if (*fmt == '%') {
            buf_putc(&out, &left, '%', &written);
            fmt++;
            continue;
        }

        switch (*fmt) {
            case 's': {
                const char *s = va_arg(args, const char *);
                if (!s) s = "(null)";
                buf_puts(&out, &left, s, &written);
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                buf_putc(&out, &left, c, &written);
                break;
            }
            case 'd':
            case 'i': {
                int v = va_arg(args, int);
                buf_putd(&out, &left, v, &written);
                break;
            }
            case 'u': {
                unsigned int v = va_arg(args, unsigned int);
                buf_putu(&out, &left, v, 10, &written);
                break;
            }
            case 'x': {
                unsigned int v = va_arg(args, unsigned int);
                buf_putu(&out, &left, v, 16, &written);
                break;
            }
            default:
                buf_putc(&out, &left, '%', &written);
                buf_putc(&out, &left, *fmt, &written);
                break;
        }
        fmt++;
    }

    if (size > 0) {
        *out = 0;
    }

    va_end(args);
    return (int)written;
}