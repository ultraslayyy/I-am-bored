#pragma once

#include <stddef.h>

typedef struct {
    void *memory;
    
    size_t size;
    size_t used;
} arena_t;

arena_t *arena_create (size_t   size);
void    *arena_alloc  (arena_t *arena, size_t size);
void     arena_destroy(arena_t *arena);