#include <lib/stddef.h>
#include <lib/stdlib.h>
#include "task.h"

extern uint32_t kernel_stack_end;

task_t *current_task = NULL;
task_t *task_list = NULL;

void task_create(void (*entry)()) {
    task_t *t = malloc(sizeof(task_t));
    uint32_t *stack = malloc(4096); // 4 KB stack
    if (!t || !stack) return;

    stack = (uint32_t *)((uint32_t)stack + 4096); // Top of 4 KB stack

    // Hardware iret frame
    *(--stack) = 0x202;           // EFLAGS
    *(--stack) = 0x08;            // CS
    *(--stack) = (uint32_t)entry; // EIP

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

    if (!task_list) {
        task_list = t;
        t->next = t;
    } else {
        t->next = task_list->next;
        task_list->next = t;
    }
}

uint32_t schedule(uint32_t esp) {
    if (!current_task) return esp;

    current_task->esp = esp;
    current_task = current_task->next;

    return current_task->esp;
}

void scheduler_init() {
    task_t *kernel_task = malloc(sizeof(task_t));
    if (!kernel_task) return;

    kernel_task->esp = (uint32_t)&kernel_stack_end; // Will be set on first schedule
    kernel_task->next = kernel_task;
    
    task_list = kernel_task;
    current_task = kernel_task;
}