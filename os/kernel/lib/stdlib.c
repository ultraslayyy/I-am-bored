#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "limits.h"

#define HEAP_START 0x01000000
#define HEAP_SIZE  (1024 * 1024)

typedef struct block {
    size_t size;
    int free;
    struct block *next;
} block_t;

static uint8_t *heap_start = (uint8_t *)HEAP_START;
static uint8_t *heap_end = (uint8_t *)(HEAP_START + HEAP_SIZE);
static block_t *free_list = NULL;

#define ALIGN8(x) (((x) + 7) & ~7)

static block_t *find_free_block(size_t size, block_t **prev) {
    block_t *current = free_list;
    *prev = NULL;

    while (current) {
        if (current->free && current->size >= size) {
            return current;
        }
        *prev = current;
        current = current->next;
    }

    return NULL;
}

void *malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    if (size > SIZE_MAX - sizeof(block_t)) {
        return NULL;
    }

    size = ALIGN8(size);

    block_t *prev = NULL;
    block_t *block = find_free_block(size, &prev);

    if (block) {
        block->free = 0;

        if (block->size >= size + sizeof(block_t) + 8) {
            block_t *new_block = (block_t *)((uint8_t *)block + sizeof(block_t) + size);
            new_block->size = block->size - size - sizeof(block_t);
            new_block->free = 1;
            new_block->next = block->next;

            block->size = size;
            block->next = new_block;
        }

        return (uint8_t *)block + sizeof(block_t);
    }

    if (heap_start + sizeof(block_t) + size > heap_end) {
        return NULL;
    }

    block = (block_t *)heap_start;
    block->size = size;
    block->free = 0;
    block->next = NULL;

    heap_start += sizeof(block_t) + size;

    if (prev) {
        prev->next = block;
    } else {
        free_list = block;
    }

    return (uint8_t *)block + sizeof(block_t);
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }

    if (size == 0) {
        free(ptr);
        return NULL;
    }

    size = ALIGN8(size);

    block_t *block = (block_t *)((uint8_t *)ptr - sizeof(block_t));

    if (block->size >= size) {
        return ptr;
    }

    if (block->next && block->next->free && block->size + sizeof(block_t) + block->next->size >= size) {
        block->size += sizeof(block_t) + block->next->size;
        block->next = block->next->next;
        return ptr;
    }

    void *new_ptr = malloc(size);
    if (!new_ptr) {
        return NULL;
    }

    memcpy(new_ptr, ptr, block->size);
    free(ptr);

    return new_ptr;
}

void free(void *ptr) {
    if (!ptr) return;

    block_t *block = (block_t *)((uint8_t *)ptr - sizeof(block_t));
    block->free = 1;

    while (block->next && block->next->free) {
        block->size += sizeof(block_t) + block->next->size;
        block->next = block->next->next;
    }

    block_t *current = free_list;
    block_t *prev = NULL;

    while (current && current != block) {
        prev = current;
        current = current->next;
    }

    if (prev && prev->free) {
        prev->size += sizeof(block_t) + block->size;
        prev->next = block->next;
    }
}

int atoi(const char *s) {
    int sign = 1;
    int result = 0;

    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r' || *s == '\f' || *s == '\v') s++;

    if (*s == '-' || *s == '+') {
        if (*s == '-') {
            sign = -1;
        }
        s++;
    }

    while (*s >= '0' && *s <= '9') {
        int digit = *s - '0';

        if (result > (INT_MAX - digit) / 10) {
            return sign == 1 ? INT_MAX : INT_MIN;
        }

        result = result * 10 + digit;
        s++;
    }
    
    return sign * result;
}

int abs(int n) {
    return (n < 0) ? -n : n;
}