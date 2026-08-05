#ifndef RUNA_VERSION_H
#define RUNA_VERSION_H

#include "export.h"
#include "types.h"

#define RUNA_VERSION_MAJOR  0
#define RUNA_VERSION_MINOR  1
#define RUNA_VERSION_PATCH  0

#define RUNA_VERSION_STRING "0.1.0"

typedef struct runa_version_t {
    runa_u32 major;
    runa_u32 minor;
    runa_u32 patch;
} runa_version_t;

#ifdef __cplusplus
extern "C" {
#endif

RUNA_API runa_version_t runa_get_version(void);

#ifdef __cplusplus
}
#endif

#endif // RUNA_VERSION_H