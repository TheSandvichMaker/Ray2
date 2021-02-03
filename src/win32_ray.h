#ifndef WIN32_RAY_H
#define WIN32_RAY_H

#include "ray_platform.h"
#include <windows.h>

global platform_api G_Platform;

#include "ray_arena.h"

typedef struct win32_allocation_header
{
    struct win32_allocation_header *Next, *Prev;
    usize Size;
    char *Base;
    u32 Flags;
    const char *Tag;
} win32_allocation_header;

typedef struct win32_state
{
    win32_allocation_header AllocationSentinel;
} win32_state;

#endif /* WIN32_RAY_H */
