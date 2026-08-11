#include <lib/stdarg.h>
#include <lib/stdint.h>
#include "string.h"

#define __fallback __attribute__((weak))

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

char *strcpy(char *dst, const char *src) {
    char *ret = dst;
    while ((*dst++ = *src++));
    return ret;
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
    for (size_t i = 0; i < copy_len; ++i) {
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

/*
static uint64_t udiv64_u32(uint64_t n, uint32_t d, uint32_t *rem) {
    uint64_t q = 0;
    uint32_t r = 0;

    for (int i = 63; i >= 0; --i) {
        r = (r << 1) | ((n >> 1) & 1);

        if (r >= d) {
            r -= d;
            q |= (uint64_t)1 << i;
        }
    }

    *rem = r;
    return q;
}

static void buf_putu64(char **buf, size_t *left, uint64_t val, int base, size_t *written) {
    char tmp[64];
    int i = 0;

    if (val == 0) {
        buf_putc(buf, left, '0', written);
        return;
    }

    while (val && i < (int)sizeof(tmp)) {
        uint32_t digit;
        val = udiv64_u32(val, (uint32_t)base, &digit);

        tmp[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
    }

    while (i--) {
        buf_putc(buf, left, tmp[i], written);
    }
} */

static void buf_putd(char **buf, size_t *left, int val, size_t *written) {
    unsigned int u;

    if (val < 0) {
        buf_putc(buf, left, '-', written);
        u = (unsigned int)(-(long long)val);
    } else {
        u = (unsigned int)val;
    }

    buf_putu(buf, left, u, 10, written);
}

int snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char *out = buf;
    size_t left = size;
    size_t written = 0;

    if (!buf) {
        left = 0;
    }

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

        /*int long_long = 0;

        if (fmt[0] == 'l' && fmt[1] == 'l') {
            long_long = 1;
            fmt += 2;
        } */

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
                /*if (long_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    buf_putu64(&out, &left, v, 10, &written);
                } else { */
                    unsigned int v = va_arg(args, unsigned int);
                    buf_putu(&out, &left, v, 10, &written);
                //}
                break;
            }
            case 'x': {
                //if (long_long) {
                //    uint64_t v = va_arg(args, uint64_t);
                //    buf_putu64(&out, &left, v, 16, &written);
                //} else {
                    unsigned int v = va_arg(args, unsigned int);
                    buf_putu(&out, &left, v, 16, &written);
                //}
                break;
            }
            default:
                buf_putc(&out, &left, '%', &written);
                buf_putc(&out, &left, *fmt, &written);
                break;
        }
        fmt++;
    }

    if (size > 0 && buf) {
        if (left > 0) {
            *out = 0;
        } else {
            buf[size - 1] = 0;
        }
    }

    va_end(args);
    return (int)written;
}

__fallback void *memcpy(void *restrict dst, const void *restrict src, size_t size) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;

    while (size && ((uintptr_t)d & 3)) {
        *d++ = *s++;
        size--;
    }

    uint32_t *d32 = (uint32_t *)d;
    const uint32_t *s32 = (const uint32_t *)s;
    while (size >= 4) {
        *d32++ = *s32++;
        size -= 4;
    }

    d = (uint8_t *)d32;
    s = (const uint8_t *)s32;
    while (size--) {
        *d++ = *s++;
    }

    return dst;
}

__fallback void *memset(void *dst, int val, size_t size) {
    uint8_t *d = (uint8_t *)dst;
    uint8_t  v = (uint8_t)val;

    // For tiny buffers just loop
    if (size < 16) {
        for (size_t i = 0; i < size; ++i) {
            d[i] = v;
        }
        return dst;
    }

    // Align to 4 bytes (word)
    while (size && ((uintptr_t)d & 3)) {
        *d++ = v;
        size--;
    }

    // Word fill
    // woohoo `movzx eax, dil` `imul eax, eax, 0x01010101`
    // same as `uint32_t v32 = (uint32_t)v * 16843009;`
    // I'm ranting
    uint32_t v32 = v | (v << 8) | (v << 16) | (v << 24);

    uint32_t *d32 = (uint32_t *)d;
    while (size >= 4) {
        *d32++ = v32;
        size -= 4;
    }

    d = (uint8_t *)d32;

    while (size--) {
        *d++ = v;
    }

    return dst;
}

__fallback void *memmove(void *dst, const void *src, size_t size) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;

    if (d < s) {
        for (size_t i = 0; i < size; ++i) {
            d[i] = s[i];
        }
    } else if (d > s) {
        for (size_t i = size; i > 0; --i) {
            d[i - 1] = s[i - 1];
        }
    }

    return dst;
}

__fallback int memcmp(const void *buf1, const void *buf2, size_t size) {
    const uint8_t *pb1 = (const uint8_t *)buf1;
    const uint8_t *pb2 = (const uint8_t *)buf2;

    // Align to 4-byte boundary
    const uint32_t *pb1_32 = (const uint32_t *)pb1;
    const uint32_t *pb2_32 = (const uint32_t *)pb2;

    while (size >= 4) {
        if (*pb1_32 != *pb2_32) {
            break;
        }
        pb1_32++;
        pb2_32++;
        size -= 4;
    }

    pb1 = (const uint8_t *)pb1_32;
    pb2 = (const uint8_t *)pb2_32;
    while (size--) {
        if (*pb1 != *pb2) {
            return (int)*pb1 - (int)*pb2;
        }
        pb1++;
        pb2++;
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