#ifndef RAY_PLATFORM_H
#define RAY_PLATFORM_H

#include <stdint.h>
#include <stddef.h>
#include <emmintrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>

#ifdef __cplusplus
extern "C"
{
#endif

//
// Types
//

struct arena;

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

struct string_u8
{
    usize Count;
    u8 *Data;
};

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

#define global static
#define internal static
#define local_persist static

#define StaticAssert(expression, message) static_assert(expression, message)

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

#define Swap(A, B) do { auto SwapTemp_ = A; A = B; B = SwapTemp_; } while(0)

#if DEBUG_BUILD
#define always_inline
#else
#define always_inline __attribute__((always_inline))
#endif

//
// Assert
//

#include <assert.h>
#define Assert(X) assert(X)

//
// Platform API
//

#define FILE_AND_LINE_STRING__(File, Line) File ":" #Line
#define FILE_AND_LINE_STRING_(File, Line) FILE_AND_LINE_STRING__(File, Line)
#define FILE_AND_LINE_STRING FILE_AND_LINE_STRING_(__FILE__, __LINE__)
#define LOCATION_STRING(...) FILE_AND_LINE_STRING " (" __VA_ARGS__ ")"

typedef struct app_pixel
{
    f32 r, g, b, w;
} app_pixel;

typedef struct app_imagebuffer
{
    u32 W, H;
    app_pixel *Backbuffer;
    app_pixel *Frontbuffer;
} app_imagebuffer;

typedef struct platform_semaphore_handle
{
    void *Opaque;
} platform_semaphore_handle;

typedef struct platform_thread_handle
{
    void *Opaque;
} platform_thread_handle;

typedef void (*platform_thread_proc)(void *UserData, platform_semaphore_handle ParentSemaphore);

enum
{
    MemFlag_NoLeakCheck = 0x1,
};

typedef struct platform_api
{
    void *(*Reserve)(usize Size, u32 Flags, const char *Tag);
    void *(*Commit)(usize Size, void *Pointer);
    void *(*Allocate)(usize Size, u32 Flags, const char *Tag);
    void (*Deallocate)(void *Pointer);
    string_u8 (*ReadEntireFile)(arena *Arena, const char *FileName);
    bool (*WriteEntireFile)(const char *FileName, string_u8 Data);
    platform_thread_handle (*CreateThread)(platform_thread_proc Proc, void *UserData);
    platform_semaphore_handle (*CreateSemaphore)(int InitialCount, int MaxCount);
    void (*WaitOnSemaphore)(platform_semaphore_handle Handle);
    void (*ReleaseSemaphore)(platform_semaphore_handle Handle, int Count, int *PreviousCount);
    usize PageSize;
    u32 LogicalCoreCount;
} platform_api;

typedef struct platform_render_settings
{
    int DEBUGShowBloomTexture;
} platform_render_settings;

//
// App API
//

typedef enum app_button_type
{
    AppButton_Left,
    AppButton_Right,
    AppButton_Forward,
    AppButton_Back,
    AppButton_Up,
    AppButton_Down,
    AppButton_LeftMouse,
    AppButton_RightMouse,
    AppButton_COUNT,
} app_button_type;

typedef struct app_button
{
    u32 HalfTransitionCount;
    u32 EndedDown;
} app_button;

static inline bool
ButtonPressed(app_button *Button)
{
    bool Result = (((Button->HalfTransitionCount == 1) && Button->EndedDown) ||
                   (Button->HalfTransitionCount > 1));
    return Result;
}

static inline bool
ButtonReleased(app_button *Button)
{
    bool Result = (((Button->HalfTransitionCount == 1) && !Button->EndedDown) ||
                   (Button->HalfTransitionCount > 1));
    return Result;
}

typedef struct app_input
{
    f32 FrameTime;
    b32 ExitRequested; 
    b32 CaptureCursor;
    f32 MouseDeltaX;
    f32 MouseDeltaY;
    app_button Buttons[AppButton_COUNT];
} app_input;

typedef struct app_init_params
{
    const char *WindowTitle;
    int WindowX, WindowY;
    int WindowW, WindowH;
} app_init_params;

typedef struct app_links
{
    void (*AppInit)(app_init_params *Params);
    void (*AppTick)(platform_api PlatformAPI, app_input *Input, app_imagebuffer *ImageBuffer);
    void (*AppExit)(void);
} app_links;

app_links AppLinks(void);

//
// Threading
//

#define MEMORY_BARRIER __atomic_thread_fence(__ATOMIC_ACQ_REL)

internal inline u32
AtomicAddU32(volatile u32 *Dest, s32 Value) {
    u32 Result = __atomic_fetch_add(Dest, Value, __ATOMIC_ACQ_REL);
    return Result;
}

internal inline u64
AtomicAddU64(volatile u64 *Dest, s64 Value) {
    u64 Result = __atomic_fetch_add(Dest, Value, __ATOMIC_ACQ_REL);
    return Result;
}

internal inline s32
AtomicAddS32(volatile s32 *Dest, s32 Value) {
    s32 Result = __atomic_fetch_add(Dest, Value, __ATOMIC_ACQ_REL);
    return Result;
}

internal inline s64
AtomicAddS64(volatile s64 *Dest, s64 Value) {
    s64 Result = __atomic_fetch_add(Dest, Value, __ATOMIC_ACQ_REL);
    return Result;
}

typedef struct ticket_mutex
{
    volatile u32 Ticket;
    volatile u32 Serving;
} ticket_mutex;

internal inline void
BeginTicketMutex(ticket_mutex *Mutex)
{
    u32 Ticket = AtomicAddU32(&Mutex->Ticket, 1);
    while (Ticket != Mutex->Serving)
    {
        _mm_pause();
    }
}

internal inline void
EndTicketMutex(ticket_mutex *Mutex)
{
    AtomicAddU32(&Mutex->Serving, 1);
}

#ifdef __cplusplus
}
#endif

#endif /* RAY_PLATFORM_H */
