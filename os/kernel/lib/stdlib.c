#include <lib/stdint.h>
#include "string.h"

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

void *malloc(size_t size) {
    if (size == 0) return 0;
    size = ALIGN8(size);

    block_t *current = free_list;
    block_t *prev = NULL;

    while (current) {
        if (current->free && current->size >= size) {
            current->free = 0;
            return (uint8_t *)current + sizeof(block_t);
        }
        prev = current;
        current = current->next;
    }

    if ((heap_start + sizeof(block_t) + size) > heap_end) {
        return NULL;
    }

    block_t *block = (block_t *)heap_start;
    block->size = size;
    block->free = 0;
    block->next = NULL;

    heap_start += sizeof(block_t) + size;

    if (prev)
        prev->next = block;
    else
        free_list = block;

    memset((uint8_t *)block + sizeof(block_t), 0, size);
    return (uint8_t *)block + sizeof(block_t);
}

void free(void *ptr) {
    if (!ptr) return;

    block_t *block = (block_t *)((uint8_t *)ptr - sizeof(block_t));
    block->free = 1;
}