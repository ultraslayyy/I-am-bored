#pragma once
#include <stddef.h>

typedef struct {
    void  *data;
    size_t length;
    size_t capacity;
    size_t elem_size;
} Vector;

void vector_init(Vector *v, size_t elem_size);
void vector_push(Vector *v, const void *elem);
void *vector_get(Vector *v, size_t index);
void vector_free(Vector *v);