#ifndef STRING_H
#define STRING_H

#include <stddef.h>

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
char *strchr(const char *s, char c);
void strcpy(char *dst, const char *src);
int snprintf(char *buf, size_t size, const char *fmt, ...);

#endif