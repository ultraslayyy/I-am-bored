#ifndef STRING_H
#define STRING_H

#include <lib/stddef.h>

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

void *memcpy(void *dst, const void *src, size_t size);
void *memset(void *dst, int val, size_t size);
int memcmp(const void *buf1, const void *buf2, size_t size);
size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
char *strchr(const char *s, char c);
void strcpy(char *dst, const char *src);
size_t strlcpy(char *dst, const char *src, size_t size);
int snprintf(char *buf, size_t size, const char *fmt, ...);
char *strktok(char **str, const char *delim);

#endif