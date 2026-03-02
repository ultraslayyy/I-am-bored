#include <lib/stdarg.h>
#include "string.h"
#include <lib/stdint.h>

size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) ++len;
    return len;
}

int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) {
        a++;
        b++;
    }

    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if (a[i] != b[i] || a[i] == 0 || b[i] == 0)  {
            return (unsigned char)a[i] - (unsigned char)b[i];
        }
    }
    return 0;
}

char *strchr(const char *s, char c) {
    while (*s) {
        if (*s == c) return (char *)s;
        s++;
    }
    return (c== 0) ? (char *)s : 0;
}

char *strrchr(const char *s, char c) {
    const char *last = 0;
    do { // Do while instead of just while so it runs at least once 
        if (*s == c) last = s;
    } while (*s++);

    return (char *)last;
}

void strcpy(char *dst, const char *src) {
    while ((*dst++ = *src++));
}

size_t strlcpy(char *dst, const char *src, size_t size) {
    size_t i = 0;

    if (size > 0) {
        for (; i < size - 1 && src[i]; ++i) {
            dst[i] = src[i];
        }
        dst[i] = '\0';
    }

    while (src[i]) i++;
    return i;
}

size_t strlcat(char *dst, const char *src, size_t size) {
    size_t dst_len = 0;
    size_t src_len = 0;

    while (dst_len < size && dst[dst_len] != '\0') {
        dst_len++;
    }

    while (src[src_len]) {
        src_len++;
    }

    if (dst_len == size) {
        return size + src_len;
    }

    size_t copy_len = (size - dst_len - 1 < src_len) ? size - dst_len - 1 : src_len;
    for (size_t i = 0; i < copy_len; i++) {
        dst[dst_len + i] = src[i];
    }

    dst[dst_len + copy_len] = '\0';

    return dst_len + src_len;
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

void *memcpy(void *dst, const void *src, size_t size) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;

    for (size_t i = 0; i < size; ++i) {
        d[i] = s[i];
    }
    return dst;
}

void *memset(void *dst, int val, size_t size) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *v = (const unsigned char *)val;

    for (size_t i = 0; i < size; ++i) {
        d[i] = *v;
    }
    return dst;
}

int memcmp(const void *buf1, const void *buf2, size_t size) {
    const unsigned char *pb1 = (const unsigned char *)buf1;
    const unsigned char *pb2 = (const unsigned char *)buf2;

    for (size_t i = 0; i < size; ++i) {
        if (pb1[i] != pb2[i]) {
            return (int)pb1[i] - (int)pb2[i];
        }
    }

    return 0;
}

char *strktok(char **str, const char *delim) {
    char *start;
    char *end;

    if (!str || !*str) {
        return 0;
    }

    start = *str;
    while (*start) {
        const char *d = delim;
        int is_delim = 0;

        while (*d) {
            if (*start == *d) {
                is_delim = 1;
                break;
            }
            d++;
        }

        if (!is_delim) {
            break;
        }

        start++;
    }

    if (*start == 0) {
        *str = 0;
        return 0;
    }

    end = start;
    while (*end) {
        const char *d = delim;
        while (*d) {
            if (*end == *d) {
                break;
            }
            d++;
        }
        if (*d) break;
        end++;
    }

    if (*end) {
        *end = 0;
        *str = end + 1;
    } else {
        *str = 0;
    }

    return start;
}