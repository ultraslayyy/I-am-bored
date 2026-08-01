#include <stdlib.h>
#include <stdio.h>
#include "sa/source.h"

int source_load(SourceFile *src, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    src->length = ftell(f);
    fseek(f, 0, SEEK_SET);

    src->contents = malloc(src->length + 1);
    fread(src->contents, 1, src->length, f);
    src->contents[src->length] = '\0';

    src->path = (char *)path;

    fclose(f);
    return 1;
}

void source_free(SourceFile *src) {
    free(src->contents);
    src->contents = NULL;
    src->length   = 0;
}