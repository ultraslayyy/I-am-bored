#ifndef RUNA_CONFIG_H
#define RUNA_CONFIG_H

#include "types.h"

typedef struct runa_config_t {
    runa_bool enable_logging;
    runa_bool enable_assertions;
    runa_bool enable_memory_tracking;
} runa_config_t;

#endif