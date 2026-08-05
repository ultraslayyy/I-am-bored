#ifndef RUNA_LOGGER_H
#define RUNA_LOGGER_H

#include "export.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum runa_log_level_t {
    RUNA_LOG_TRACE,
    RUNA_LOG_INFO,
    RUNA_LOG_WARNING,
    RUNA_LOG_ERROR,
    RUNA_LOG_FATAL
} runa_log_level_t;

RUNA_API void runa_log(runa_log_level_t level, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif // RUNA_LOGGER_H