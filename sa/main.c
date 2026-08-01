#include <stdio.h>
#include "sa/arena.h"
#include "sa/source.h"
#include "sa/vector.h"

int main(void) {
    Arena *arena = arena_create(1024);

    SourceFile file;
    if (!source_load(&file, "test.ts")) {
        printf("Failed to load file\n");
        return 1;
    }

    printf("Loaded %zu bytes\n", file.length);

    Vector v;
    vector_init(&v, sizeof(int));

    int x = 42;
    vector_push(&v, &x);

    int *p = vector_get(&v, 0);
    printf("Vector[0] = %d\n", *p);

    vector_free(&v);
    source_free(&file);
    arena_destroy(arena);

    return 0;
}