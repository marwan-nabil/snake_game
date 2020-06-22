#pragma once
#include <stdint.h>

#define Assert(cond) if(!(cond)){*((int *) 0) = 0;}

typedef float f32;
typedef double f64;
typedef int32_t i32;
typedef uint32_t u32;
typedef uint8_t u8;
typedef int64_t i64;
typedef uint64_t u64;
