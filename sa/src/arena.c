#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sa/arena.h"

typedef struct Chunk {
    struct Chunk *next;
    size_t  size;
    size_t  used;
    uint8_t data[];
} Chunk;

struct Arena {
    Chunk *head;
};

static Chunk *chunk_create(size_t size) {
    Chunk *c = malloc(sizeof(Chunk) + size);
    c->next  = NULL;
    c->size  = size;
    c->used  = 0;
    return c;
}

Arena *arena_create(size_t initial_size) {
    Arena *a = malloc(sizeof(Arena));
    a->head  = chunk_create(initial_size);
    return a;
}

void *arena_alloc(Arena *arena, size_t size) {
    Chunk *c = arena->head;

    if (c->used + size > c->size) {
        size_t new_size = c->size * 2;
        if (new_size < size) {
            new_size = size;
        }

        Chunk *newc = chunk_create(new_size);
        newc->next  = arena->head;
        arena->head = newc;
        c = newc;
    }

    void *ptr = c->data + c->used;
    c->used += size;
    return ptr;
}

void arena_reset(Arena *arena) {
    Chunk *c = arena->head->next;

    while (c) {
        Chunk *next = c->next;
        free(c);
        c = next;
    }

    arena->head->used = 0;
    arena->head->next = NULL;
}

void arena_destroy(Arena *arena) {
    Chunk *c = arena->head;

    while (c) {
        Chunk *next = c->next;
        free(c);
        c = next;
    }

    free(arena);
}

