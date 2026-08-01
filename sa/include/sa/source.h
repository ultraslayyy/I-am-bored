#pragma once
#include <stddef.h>

typedef struct {
    char  *path;
    char  *contents;
    size_t length;
} SourceFile;

int  source_load(SourceFile *src, const char *path);
void source_free(SourceFile *src);