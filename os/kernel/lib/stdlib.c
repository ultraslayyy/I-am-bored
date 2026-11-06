#include <arch.h>
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

#define ALIGN8(x) (((x) + 7) & ~7)

#define BLOCK_HEADER_SIZE ALIGN8(sizeof(block_t))

static uint8_t *heap_start = (uint8_t *)HEAP_START;
static uint8_t *heap_end = (uint8_t *)(HEAP_START + HEAP_SIZE);
static block_t *free_list = NULL;

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
    uint32_t flags = irq_save();

    if (size == 0) {
        irq_restore(flags);
        return NULL;
    }

    if (size > SIZE_MAX - BLOCK_HEADER_SIZE) {
        irq_restore(flags);
        return NULL;
    }

    size = ALIGN8(size);

    block_t *prev = NULL;
    block_t *block = find_free_block(size, &prev);

    if (block) {
        block->free = 0;

        if (block->size >= size + BLOCK_HEADER_SIZE + 8) {
            block_t *new_block = (block_t *)((uint8_t *)block + BLOCK_HEADER_SIZE + size);
            new_block->size = block->size - size - BLOCK_HEADER_SIZE;
            new_block->free = 1;
            new_block->next = block->next;

            block->size = size;
            block->next = new_block;
        }

        irq_restore(flags);
        return (uint8_t *)block + BLOCK_HEADER_SIZE;
    }

    if (heap_start + BLOCK_HEADER_SIZE + size > heap_end) {
        irq_restore(flags);
        return NULL;
    }

    block = (block_t *)heap_start;
    block->size = size;
    block->free = 0;
    block->next = NULL;

    heap_start += BLOCK_HEADER_SIZE + size;

    if (prev) {
        prev->next = block;
    } else {
        free_list = block;
    }

    irq_restore(flags);
    return (uint8_t *)block + BLOCK_HEADER_SIZE;
}

void *realloc(void *ptr, size_t size) {
    uint32_t flags = irq_save();

    if (ptr == NULL) {
        irq_restore(flags);
        return malloc(size);
    }

    if (size == 0) {
        free(ptr);
        irq_restore(flags);
        return NULL;
    }

    size = ALIGN8(size);

    block_t *block = (block_t *)((uint8_t *)ptr - BLOCK_HEADER_SIZE);

    if (block->size >= size) {
        irq_restore(flags);
        return ptr;
    }

    if (block->next && block->next->free && block->size + BLOCK_HEADER_SIZE + block->next->size >= size) {
        block->size += BLOCK_HEADER_SIZE + block->next->size;
        block->next = block->next->next;
        irq_restore(flags);
        return ptr;
    }

    void *new_ptr = malloc(size);
    if (!new_ptr) {
        irq_restore(flags);
        return NULL;
    }

    memcpy(new_ptr, ptr, block->size);
    free(ptr);

    irq_restore(flags);
    return new_ptr;
}

void free(void *ptr) {
    if (!ptr) return;

    uint32_t flags = irq_save();

    block_t *block = (block_t *)((uint8_t *)ptr - BLOCK_HEADER_SIZE);
    block->free = 1;

    while (block->next && block->next->free) {
        block->size += BLOCK_HEADER_SIZE + block->next->size;
        block->next = block->next->next;
    }

    block_t *current = free_list;
    block_t *prev = NULL;

    while (current && current != block) {
        prev = current;
        current = current->next;
    }

    if (prev && prev->free) {
        prev->size += BLOCK_HEADER_SIZE + block->size;
        prev->next = block->next;
    }

    irq_restore(flags);
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