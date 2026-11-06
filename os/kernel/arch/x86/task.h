#pragma once
#include <lib/stdint.h>

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_TERMINATING,
    TASK_ZOMBIE
} task_state_t;

typedef void (*task_cleanup_t)(void);

typedef struct task {
    uint32_t esp;
    uint32_t pid;

    void (*entry)(void *);
    void *arg;
    void (*arg_cleanup)(void *);

    task_cleanup_t cleanup;

    void *stack_base;

    task_state_t state;

    struct task *next;
} task_t;

extern task_t *current_task;
extern task_t *task_list;

extern uint32_t foreground_task;

void scheduler_init(void);
uint32_t schedule(uint32_t esp);

uint32_t task_create(void (*entry)(void *), void *arg, void (*arg_cleanup)(void *));

void task_exit(void);
void task_terminate(uint32_t pid);

task_t *task_get(uint32_t pid);
int task_exists(uint32_t pid);

void task_set_cleanup(task_cleanup_t cleanup);