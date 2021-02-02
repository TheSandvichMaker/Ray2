#ifndef RAY_PLATFORM_H
#define RAY_PLATFORM_H

#include <stdint.h>
#include <stddef.h>

//
// Types
//

typedef int8_t   b8;
typedef int16_t  b16;
typedef int32_t  b32;

typedef float    f32;
typedef double   f64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef uintptr_t usize;
typedef intptr_t  ssize;

//
// Constants
//

#define F32_MAX           3.402823466e+38f
#define F32_MIN           -F32_MAX
#define F32_SMALLEST      1.175494351e-38f
#define F32_TRUE_SMALLEST 1.401298464e-45f
#define F32_EPSILON       1.192092896e-07f

#define F64_MAX           1.7976931348623158e+308
#define F64_MIN           -F64_MAX
#define F64_SMALLEST      2.2250738585072014e-308
#define F64_TRUE_SMALLEST 4.9406564584124654e-324
#define F64_EPSILON       2.2204460492503131e-016

#define S8_MIN            (-127i8 - 1)
#define S16_MIN           (-32767i16 - 1)
#define S32_MIN           (-2147483647i32 - 1)
#define S64_MIN           (-9223372036854775807i64 - 1)
#define S8_MAX            127i8
#define S16_MAX           32767i16
#define S32_MAX           2147483647i32
#define S64_MAX           9223372036854775807i64
#define U8_MAX            0xffui8
#define U16_MAX           0xffffui16
#define U32_MAX           0xffffffffui32
#define U64_MAX           0xffffffffffffffffui64

#define true  1
#define false 0

#define auto __auto_type
#define bool _Bool

#define StaticAssert(expression, message) _Static_assert(expression, message)

#define INVALID_CODE_PATH Assert(!"Invalid Code Path");
#define INVALID_DEFAULT_CASE default: { Assert(!"Invalid Default Case"); } break;

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))
#define ArrayEnd(array) (array) + ArrayCount(array)

#define SllStackPush(h, n) ((n)->next = (h), (h) = (n))
#define SllStackPop_(h) ((h) = (h)->next)
#define SllStackPop(h) h; SllStackPop_(h)
#define SllQueuePush(f, l, n) ((n)->next = 0, ((f) ? (l)->next = (n) : (f) = (n)), (l) = (n))
#define SllQueuePop(f, l, n) f; (SllStackPop_(f), ((f) ? 0 : (l) = 0))

#define ForSllUnique(it, head, next) for (auto* it = head; it; it = it->next)
#define ForSll(it, head) ForSllUnique(it, head, next)
#define ForSllOuterUnique(it_at, head, next) for (auto** it_at = &(head); *it_at; it_at = &(*it_at)->next)
#define ForSllOuter(it_at, head) ForSllOuterUnique(it_at, head, next)

#define DllInit(s) ((s)->next = s, (s)->prev = s)
#define DllInsertFront(h, n) ((n)->next = (h)->next, (n)->prev = h, (n)->next->prev = n, (n)->prev->next = n)
#define DllInsertBack(h, n) ((n)->next = h, (n)->prev = (h)->prev, (n)->next->prev = n, (n)->prev->next = n)
#define DllRemove(n) ((n)->next->prev = (n)->prev, (n)->prev->next = (n)->next)
#define DllIsEmpty(s) ((s)->next == (s))

#define ForDllUnique(it, sentinel, next, prev) for (auto prev_##it = (sentinel), it = (sentinel)->next; \
                                                    it != (sentinel);                                   \
                                                    prev_##it = prev_##it->next, it = prev_##it->next)
#define ForDll(it, sentinel) ForDllUnique(it, sentinel, next, prev)

#define Paste__(a, b) a##b
#define Paste_(a, b) Paste__(a, b)
#define Paste(a, b) Paste_(a, b)
#define Stringize__(x) #x
#define Stringize_(x) Stringize__(x)
#define Stringize(x) Stringize_(x)
#define Expand_(x) x
#define Expand(x) Expand(x)

#define BitIsSet(mask, bit) ((mask) & ((u64)1 << bit))
#define SetBit(mask, bit)   ((mask) |= ((u64)1 << bit))
#define UnsetBit(mask, bit) ((mask) &= ~((u64)1 << bit))

#define ABS(val) ((val) < 0 ? -(val) : (val))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi) (Max((lo), Min((hi), (x))))

#define AlignPow2(value, align) (((value) + ((align) - 1)) & ~((align) - 1))
#define Align4(value) ((value + 3) & ~3)
#define Align8(value) ((value + 7) & ~7)
#define Align16(value) ((value + 15) & ~15)

#define Kilobytes(num) ((num)*(u64)1024)
#define Megabytes(num) (Kilobytes(num)*(u64)1024)
#define Gigabytes(num) (Megabytes(num)*(u64)1024)
#define Terabytes(num) (Gigabytes(num)*(u64)1024)

#define Milliseconds(seconds) ((seconds) / 1000.0f)

#define U64FromPtr(p) (u64)(uintptr_t)(p)

#define alignof(x) _Alignof(x)
// #define offsetof(Type, M) (usize)((char*)&((Type*)0)->M - (char*)0)

//
// Assert
//

#include <assert.h>
#define Assert(X) assert(X)

//
// Platform API
//

typedef struct platform_api {
    void* (*Allocate)(usize Size);
    void* (*Reserve)(usize Size);
    void* (*Commit)(usize Size, void* Pointer);
    void  (*Deallocate)(void* Pointer);
    usize PageSize;
} platform_api;

//
// App API
//

typedef struct app_input {
    b32 ExitRequested; 
} app_input;

typedef struct app_init_params {
    void (*AppTick)(platform_api PlatformAPI, app_input* Input);
} app_init_params;

app_init_params AppEntry(void);

#endif /* RAY_PLATFORM_H */
