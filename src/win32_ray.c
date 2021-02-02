#include "win32_ray.h"

static b32 G_Running = true;

static void*
Win32Allocate(usize Size)
{
    void* Result = VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    return Result;
}

static void*
Win32Reserve(usize Size)
{
    void* Result = VirtualAlloc(0, Size, MEM_RESERVE, PAGE_NOACCESS);
    return Result;
}

static void*
Win32Commit(usize Size, void* Pointer)
{
    void* Result = VirtualAlloc(Pointer, Size, MEM_COMMIT, PAGE_READWRITE);
    return Result;
}

static void
Win32Deallocate(void* Pointer)
{
    if (Pointer)
    {
        VirtualFree(Pointer, 0, MEM_RELEASE);
    }
}

int
main(int argc, char** argv)
{
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);

    usize PageSize = SystemInfo.dwPageSize;

    LARGE_INTEGER PerfFreq;
	QueryPerformanceFrequency(&PerfFreq);

    platform_api API =
    {
        .Allocate   = Win32Allocate,
        .Reserve    = Win32Reserve,
        .Commit     = Win32Commit,
        .Deallocate = Win32Deallocate,
        .PageSize   = PageSize,
    };

    app_init_params Params = AppEntry();
    app_input Input = { 0 };

    while (G_Running)
    {
        Params.AppTick(API, &Input);
    }
}
