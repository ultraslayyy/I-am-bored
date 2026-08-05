#ifndef RUNA_ENGINE_H
#define RUNA_ENGINE_H

#include "config.h"
#include "export.h"
#include "result.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

RUNA_API runa_result_t runa_initialise(const runa_config_t *config);
RUNA_API void          runa_shutdown  (void);
RUNA_API void          runa_update    (void);
RUNA_API runa_bool     runa_is_running(void);
RUNA_API void          runa_quit      (void);

#ifdef __cplusplus
}
#endif

#endif // RUNA_ENGINE_H