#ifndef RUNA_TYPES_H
#define RUNA_TYPES_H

#include <stddef.h>
#include <stdint.h>

typedef int runa_bool;

#define RUNA_FALSE 0
#define RUNA_TRUE  1

typedef int8_t   rua_i8;
typedef uint8_t  runa_u8;

typedef int16_t  runa_i16;
typedef uint16_t runa_u16;

typedef int32_t  runa_i32;
typedef uint32_t runa_u32;

typedef int64_t  runa_i64;
typedef uint64_t runa_u64;

typedef float    runa_f32;
typedef double   runa_f64;

typedef size_t   runa_size;

#endif // RUNA_TYPES_H