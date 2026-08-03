#include <stdlib.h>
#include <string.h>
#include "arena.h"

arena_t *arena_create(size_t size) {
    arena_t *arena = malloc(sizeof(arena_t));

    arena->memory = malloc(size);

    arena->size = size;
    arena->used = 0;

    return arena;
}

void *arena_alloc(arena_t *arena, size_t size) {
    if (arena->used + size > arena->size) {
        return NULL;
    }

    void *ptr = (char *)arena->memory + arena->used;

    arena->used += size;

    memset(ptr, 0, size);

    return ptr;
}

void arena_destroy(arena_t *arena) {
    free(arena->memory);
    free(arena);
}