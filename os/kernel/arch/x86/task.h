#pragma once
#include <lib/stdint.h>

typedef struct task {
    uint32_t esp;
    struct task *next;
} task_t;

void scheduler_init(void);
uint32_t schedule(uint32_t esp);
void task_create(void (*entry)());