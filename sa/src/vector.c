#include <stdlib.h>
#include <string.h>
#include "sa/vector.h"

static void vector_grow(Vector *v) {
    size_t new_cap = v->capacity ? v->capacity * 2 : 8;

    v->data = realloc(v->data, new_cap * v->elem_size);
    v->capacity = new_cap;
}

void vector_init(Vector *v, size_t elem_size) {
    v->data      = NULL;
    v->length    = 0;
    v->capacity  = 0;
    v->elem_size = elem_size;
}

void vector_push(Vector *v, const void *elem) {
    if (v->length + 1 > v->capacity) {
        vector_grow(v);
    }

    void *target = (char *)v->data + (v->length * v->elem_size);
    memcpy(target, elem, v->elem_size);
    v->length++;
}

void *vector_get(Vector *v, size_t index) {
    return (char *)v->data + (index * v->elem_size);
}

void vector_free(Vector *v) {
    free(v->data);
    v->data   = NULL;
    v->length = v->capacity = 0;
}