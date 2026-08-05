#ifndef RUNA_MEMORY_H
#define RUNA_MEMORY_H

#include "export.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

RUNA_API void *runa_malloc (runa_size size);
RUNA_API void *runa_calloc (runa_size count,  runa_size size);
RUNA_API void *runa_realloc(void     *memory, runa_size size);
RUNA_API void  runa_free   (void     *memory);

#ifdef __cplusplus
}
#endif

#endif // RUNA_MEMORY_H