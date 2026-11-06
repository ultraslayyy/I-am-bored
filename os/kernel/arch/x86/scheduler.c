#include <lib/stddef.h>
#include <lib/stdlib.h>
#include "irq.h"
#include "task.h"

extern uint32_t kernel_stack_end;

task_t *current_task = NULL;
task_t *task_list = NULL;

static uint32_t next_pid = 1;

static void task_trampoline(void) {
    current_task->entry(current_task->arg);
    task_exit();
}

uint32_t task_create(void (*entry)(void *), void *arg, void (*arg_cleanup)(void *)) {
    if (!entry) {
        return 0;
    }

    task_t *t = malloc(sizeof(task_t));
    if (!t) {
        return 0;
    }

    void *stack_base = malloc(4096); // 4 KB stack
    if (!stack_base) {
        free(t);
        return 0;
    }

    uint32_t *stack = (uint32_t *)((uint32_t)stack_base + 4096); // Top of 4 KB stack

    t->pid         = next_pid++;
    t->entry       = entry;
    t->arg         = arg;
    t->arg_cleanup = arg_cleanup;
    t->stack_base  = stack_base;
    t->state       = TASK_READY;

    // Hardware iret frame
    *(--stack) = 0x202;                     // EFLAGS
    *(--stack) = 0x08;                      // CS
    *(--stack) = (uint32_t)task_trampoline; // EIP

    *(--stack) = 0; // EAX
    *(--stack) = 0; // ECX
    *(--stack) = 0; // EDX
    *(--stack) = 0; // EBX
    *(--stack) = 0; // ESP (dummy value)
    *(--stack) = 0; // EBP
    *(--stack) = 0; // ESI
    *(--stack) = 0; // EDI

    *(--stack) = 0x10; // DS
    *(--stack) = 0x10; // ES
    *(--stack) = 0x10; // FS
    *(--stack) = 0x10; // GS

    t->esp = (uint32_t)stack;

    // Modify list with interrupts disabled
    uint32_t flags = irq_save();

    if (!task_list) {
        task_list = t;
        t->next = t;
    } else {
        t->next = task_list->next;
        task_list->next = t;
    }

    irq_restore(flags);

    return t->pid;
}

static void task_cleanup(task_t *task) {
    if (task->cleanup) {
        task->cleanup();
        task->cleanup = NULL;
    }

    if (task->arg_cleanup) {
        task->arg_cleanup(task->arg);
        task->arg_cleanup = NULL;
    }
}

static void task_reap_dead(void) {
    if (!task_list) {
        return;
    }

    for (;;) {
        task_t *dead = NULL;
        task_t *prev = NULL;

        task_t *t = task_list;

        do {
            if(t != current_task && t->pid != 0 && (t->state == TASK_TERMINATING || t->state == TASK_ZOMBIE)) {
                dead = t;
                break;
            }

            prev = t;
            t = t->next;
        } while (t != task_list);

        if (!dead) {
            break;
        }

        prev = dead;

        while (prev->next != dead) {
            prev = prev->next;
        }

        if (dead->next == dead) {
            task_list = NULL;
        } else {
            prev->next = dead->next;

            if (task_list == dead) {
                task_list = dead->next;
            }
        }

        task_cleanup(dead);

        free(dead->stack_base);
        free(dead);
    }
}

uint32_t schedule(uint32_t esp) {
    if (!current_task) {
        return esp;
    }

    current_task->esp = esp;

    task_reap_dead();

    task_t *previous = current_task;
    task_t *next     = previous->next;

    while (next != previous) {
        if (next->state == TASK_READY || next->state == TASK_RUNNING) {
            break;
        }

        next = next->next;
    }

    if (next == previous) {
        // Should only happen if the current task is runnable
        // PID 0 should guarantee this
        return esp;
    }

    if (previous->state == TASK_RUNNING) {
        previous->state = TASK_READY;
    }

    current_task = next;
    current_task->state = TASK_RUNNING;

    return current_task->esp;
}

task_t *task_get(uint32_t pid) {
    if (!task_list) {
        return NULL;
    }

    task_t *t = task_list;

    do {
        if (t->pid == pid) {
            return t;
        }

        t = t->next;
    } while (t != task_list);

    return NULL;
}

void task_terminate(uint32_t pid) {
    if (pid == 0) {
        return;
    }

    task_t *t = task_get(pid);

    if (!t) {
        return;
    }

    if (t->state == TASK_READY || t->state == TASK_RUNNING) {
        t->state = TASK_TERMINATING;
    }
}

void task_exit(void) {
    if (!current_task || current_task->pid == 0) {
        return;
    }

    current_task->state = TASK_ZOMBIE;

    // Wait for next timer tick to switch out
    for (;;) {
        __asm__ volatile("sti; hlt");
    }
}

void task_set_cleanup(task_cleanup_t cleanup) {
    if (!current_task || current_task->pid == 0) {
        return;
    }

    current_task->cleanup = cleanup;
}

void scheduler_init() {
    task_t *kernel_task = malloc(sizeof(task_t));
    if (!kernel_task) return;

    kernel_task->esp        = (uint32_t)&kernel_stack_end; // Will be set on first schedule
    kernel_task->pid        = 0;
    kernel_task->entry      = NULL;
    kernel_task->arg        = NULL;
    kernel_task->stack_base = NULL;
    kernel_task->state      = TASK_RUNNING;
    kernel_task->next       = kernel_task;
    
    task_list = kernel_task;
    current_task = kernel_task;
}