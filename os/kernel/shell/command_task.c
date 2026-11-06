#include <lib/stdlib.h>
#include <arch.h>
#include "command_task.h"

void command_task(void *arg) {
    command_context_t *ctx = (command_context_t *)arg;

    ctx->handler(ctx->argc, ctx->argv);
    free(ctx);
    task_exit();
}