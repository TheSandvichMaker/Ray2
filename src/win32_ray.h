#ifndef WIN32_RAY_H
#define WIN32_RAY_H

#include "ray_platform.h"
#include <windows.h>
#include <gl/gl.h>
#include "external/robustwin32io.h"

#undef CreateSemaphore

global platform_api Platform;

#include "ray_handmade_math.h"
#include "ray_arena.h"
#include "ray_opengl.h"

struct win32_allocation_header
{
    win32_allocation_header *Next, *Prev;
    usize Size;
    char *Base;
    u32 Flags;
    const char *Tag;
};

struct win32_state
{
    win32_allocation_header AllocationSentinel;
};

#endif /* WIN32_RAY_H */
