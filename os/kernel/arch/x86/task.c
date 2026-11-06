#include <lib/stdlib.h>
#include "irq.h"
#include "task.h"

uint32_t foreground_task = 0;

int task_exists(uint32_t pid) {
    uint32_t flags = irq_save();

    int found = task_get(pid) != NULL;
    
    irq_restore(flags);
    return found;
}