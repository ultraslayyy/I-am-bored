#ifndef RUNA_PLATFORM_H
#define RUNA_PLATFORM_H

#include "export.h"
#include "result.h"

#ifdef __cplusplus
extern "C" {
#endif

RUNA_API runa_result_t runa_platform_initialise(void);
RUNA_API void          runa_platform_shutdown  (void);
RUNA_API void          runa_platform_update    (void);

#ifdef __cplusplus
}
#endif

#endif // RUNA_PLATFORM_H