#ifndef SI117X_TYPES
#define SI117X_TYPES
#include "types.h"
typedef signed char       s8;
typedef signed short      s16;
typedef signed int        s32;
typedef unsigned char     u8;
typedef unsigned short    u16;
typedef unsigned int      u32;

typedef signed char       S8;
typedef signed short      S16;
typedef signed int        S32;
typedef unsigned char     U8;
typedef unsigned short    U16;
typedef unsigned int      U32;

typedef void *            HANDLE;
typedef char *            STRING;
typedef s16               PT_RESULT;
typedef s8                PT_BOOL;

#define code
#define xdata

#define LSB 0
#define MSB 1
#define b0  0
#define b1  1
#define b2  2
#define b3  3

typedef char BIT;

#endif
