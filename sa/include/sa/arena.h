#pragma once
#include <stddef.h>

typedef struct Arena Arena;

Arena *arena_create (size_t initial_size);
void  *arena_alloc  (Arena *arena, size_t size);
void   arena_destroy(Arena *arena);
void   arena_reset  (Arena *arena);