#ifndef RUNA_TIMER_H
#define RUNA_TIMER_H

#include "export.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

RUNA_API runa_f64 runa_get_time        (void);
RUNA_API runa_f64 runa_get_delta_time  (void);
RUNA_API runa_f64 runa_get_elapsed_time(void);
RUNA_API void     runa_sleep           (runa_u32 milliseconds);

#ifdef __cplusplus
}
#endif

#endif // RUNA_TIMER_H