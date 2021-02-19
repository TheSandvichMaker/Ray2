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

#define ScopeExpr(Begin, End) for (int _DummyVar = (Begin, 1); _DummyVar; (End, _DummyVar = 0))

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
    u8 HalfTransitionCount;
    u8 EndedDown;
} app_button;

static inline bool
ButtonPressed(app_button Button)
{
    bool Result = (((Button.HalfTransitionCount == 1) && Button.EndedDown) ||
                   (Button.HalfTransitionCount > 1));
    return Result;
}

static inline bool
ButtonReleased(app_button Button)
{
    bool Result = (((Button.HalfTransitionCount == 1) && !Button.EndedDown) ||
                   (Button.HalfTransitionCount > 1));
    return Result;
}

typedef enum platform_key_code
{
    PKC_LButton        = 0x1,
    PKC_RButton        = 0x2,
    PKC_Cancel         = 0x3,
    PKC_MButton        = 0x4,
    PKC_XButton1       = 0x5,
    PKC_XButton2       = 0x6,
    PKC_Backspace      = 0x8,
    PKC_Tab            = 0x9,
    PKC_Clear          = 0xC,
    PKC_Return         = 0xD,
    PKC_Shift          = 0x10,
    PKC_Control        = 0x11,
    PKC_Alt            = 0x12,
    PKC_Pause          = 0x13,
    PKC_CapsLock       = 0x14,
    PKC_Kana           = 0x15,
    PKC_Hangul         = 0x15,
    PKC_Junja          = 0x17,
    PKC_Final          = 0x18,
    PKC_Hanja          = 0x19,
    PKC_Kanji          = 0x19,
    PKC_Escape         = 0x1B,
    PKC_Convert        = 0x1C,
    PKC_NonConvert     = 0x1D,
    PKC_Accept         = 0x1E,
    PKC_ModeChange     = 0x1F,
    PKC_Space          = 0x20,
    PKC_PageUp         = 0x21,
    PKC_PageDown       = 0x22,
    PKC_End            = 0x23,
    PKC_Home           = 0x24,
    PKC_Left           = 0x25,
    PKC_Up             = 0x26,
    PKC_Right          = 0x27,
    PKC_Down           = 0x28,
    PKC_Select         = 0x29,
    PKC_Print          = 0x2A,
    PKC_Execute        = 0x2B,
    PKC_PrintScreen    = 0x2C,
    PKC_Insert         = 0x2D,
    PKC_Delete         = 0x2E,
    PKC_Help           = 0x2F,
    /* 0x30 - 0x39: ascii numerals */
    /* 0x3A - 0x40: undefined */
    /* 0x41 - 0x5A: ascii alphabet */
    PKC_LSys           = 0x5B,
    PKC_RSys           = 0x5C,
    PKC_Apps           = 0x5D,
    PKC_Sleep          = 0x5f,
    PKC_Numpad0        = 0x60,
    PKC_Numpad1        = 0x61,
    PKC_Numpad2        = 0x62,
    PKC_Numpad3        = 0x63,
    PKC_Numpad4        = 0x64,
    PKC_Numpad5        = 0x65,
    PKC_Numpad6        = 0x66,
    PKC_Numpad7        = 0x67,
    PKC_Numpad8        = 0x68,
    PKC_Numpad9        = 0x69,
    PKC_Multiply       = 0x6A,
    PKC_Add            = 0x6B,
    PKC_Separator      = 0x6C,
    PKC_Subtract       = 0x6D,
    PKC_Decimal        = 0x6E,
    PKC_Divide         = 0x6f,
    PKC_F1             = 0x70,
    PKC_F2             = 0x71,
    PKC_F3             = 0x72,
    PKC_F4             = 0x73,
    PKC_F5             = 0x74,
    PKC_F6             = 0x75,
    PKC_F7             = 0x76,
    PKC_F8             = 0x77,
    PKC_F9             = 0x78,
    PKC_F10            = 0x79,
    PKC_F11            = 0x7A,
    PKC_F12            = 0x7B,
    PKC_F13            = 0x7C,
    PKC_F14            = 0x7D,
    PKC_F15            = 0x7E,
    PKC_F16            = 0x7F,
    PKC_F17            = 0x80,
    PKC_F18            = 0x81,
    PKC_F19            = 0x82,
    PKC_F20            = 0x83,
    PKC_F21            = 0x84,
    PKC_F22            = 0x85,
    PKC_F23            = 0x86,
    PKC_F24            = 0x87,
    PKC_Numlock        = 0x90,
    PKC_Scroll         = 0x91,
    PKC_LShift         = 0xA0,
    PKC_RShift         = 0xA1,
    PKC_LControl       = 0xA2,
    PKC_RControl       = 0xA3,
    PKC_LAlt           = 0xA4,
    PKC_RAlt           = 0xA5,
    /* 0xA6 - 0xAC: browser keys, not sure what's up with that */
    PKC_VolumeMute     = 0xAD,
    PKC_VolumeDown     = 0xAE,
    PKC_VolumeUp       = 0xAF,
    PKC_MediaNextTrack = 0xB0,
    PKC_MediaPrevTrack = 0xB1,
    /* 0xB5 - 0xB7: "launch" keys, not sure what's up with that */
    PKC_Oem1           = 0xBA, // misc characters, us standard: ';:'
    PKC_Plus           = 0xBB,
    PKC_Comma          = 0xBC,
    PKC_Minus          = 0xBD,
    PKC_Period         = 0xBE,
    PKC_Oem2           = 0xBF, // misc characters, us standard: '/?'
    PKC_Oem3           = 0xC0, // misc characters, us standard: '~'
    /* 0xC1 - 0xDA: reserved / unassigned */
    /* 0xDB - 0xF5: more miscellanious OEM codes I'm ommitting for now */
    /* 0xF6 - 0xF9: keys I've never heard of */
    PKC_Play           = 0xFA,
    PKC_Zoom           = 0xFB,
    PKC_OemClear       = 0xFE,
    PKC_COUNT,
} platform_key_code;

typedef struct app_key_event
{
    u8 KeyCode;
    u8 Pressed;
} app_key_event;

#define APP_KEY_EVENT_MAX 256
typedef struct app_input
{
    f32 FrameTime;
    b32 ExitRequested; 
    b32 CaptureCursor;
    s32 ClientMouseX;
    s32 ClientMouseY;
    s32 MouseDeltaX;
    s32 MouseDeltaY;
    s32 RawMouseDeltaX;
    s32 RawMouseDeltaY;
    app_button Buttons[AppButton_COUNT];

    u32 KeyEventCount;
    app_key_event KeyEvents[APP_KEY_EVENT_MAX];
} app_input;

typedef struct app_init_params
{
    const char *WindowTitle;
    int WindowX, WindowY;
    int WindowW, WindowH;
} app_init_params;

typedef struct app_render_commands
{
    usize CommandBufferSize;
    usize CommandBufferAt;
    char *CommandBuffer;
} app_render_commands;

typedef struct app_links
{
    void (*AppInit)(app_init_params *Params);
    void (*AppTick)(platform_api PlatformAPI, app_input *Input, app_imagebuffer *ImageBuffer, app_render_commands *RenderCommands);
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
