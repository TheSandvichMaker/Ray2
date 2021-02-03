#include "win32_ray.h"

#include <stdio.h>

global b32 G_Running = true;
global win32_state G_Win32State;
global LARGE_INTEGER G_PerfFreq;

internal void *
Win32Reserve(usize Size, u32 Flags, const char *Tag)
{
    usize PageSize = G_Platform.PageSize;
    usize TotalSize = PageSize + Size;

    win32_allocation_header *Header = VirtualAlloc(0, TotalSize, MEM_RESERVE, PAGE_NOACCESS);
    VirtualAlloc(Header, PageSize, MEM_COMMIT, PAGE_READWRITE);

    Header->Size = TotalSize;
    Header->Base = (char *)Header + PageSize;
    Header->Flags = Flags;
    Header->Tag = Tag;

    Header->Next = &G_Win32State.AllocationSentinel;
    Header->Prev = G_Win32State.AllocationSentinel.Prev;
    Header->Next->Prev = Header;
    Header->Prev->Next = Header;

    return Header->Base;
}

internal void *
Win32Commit(usize Size, void *Pointer)
{
    void *Result = VirtualAlloc(Pointer, Size, MEM_COMMIT, PAGE_READWRITE);
    return Result;
}

internal void *
Win32Allocate(usize Size, u32 Flags, const char *Tag)
{
    void *Result = Win32Reserve(Size, Flags, Tag);
    if (Result)
    {
        Result = Win32Commit(Size, Result);
    }
    return Result;
}

internal void
Win32Deallocate(void *Pointer)
{
    if (Pointer)
    {
        win32_allocation_header *Header = (win32_allocation_header *)((char *)Pointer - G_Platform.PageSize);
        Header->Prev->Next = Header->Next;
        Header->Next->Prev = Header->Prev;
        VirtualFree(Header, 0, MEM_RELEASE);
    }
}

internal CALLBACK LRESULT
Win32WindowProc(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = 0;
    switch (Message)
    {
        case WM_CLOSE:
        case WM_DESTROY:
        {
            G_Running = false;
        } break;

        default:
        {
            Result = DefWindowProcA(Window, Message, wParam, lParam);
        } break;
    }
    return Result;
}

int
main(int argc, char **argv)
{
    HINSTANCE Instance = 0;

    G_Win32State.AllocationSentinel.Next = &G_Win32State.AllocationSentinel;
    G_Win32State.AllocationSentinel.Prev = &G_Win32State.AllocationSentinel;

	QueryPerformanceFrequency(&G_PerfFreq);

    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);

    platform_api API =
    {
        .Reserve    = Win32Reserve,
        .Commit     = Win32Commit,
        .Allocate   = Win32Allocate,
        .Deallocate = Win32Deallocate,
        .PageSize   = SystemInfo.dwPageSize,
    };
    G_Platform = API;

    app_init_params Params =
    {
        .WindowTitle = "Unnamed Window",
        .WindowX = CW_USEDEFAULT,
        .WindowY = CW_USEDEFAULT,
        .WindowW = CW_USEDEFAULT,
        .WindowH = CW_USEDEFAULT,
    };
    AppEntry(&Params);

    WNDCLASSA WindowClass =
    {
        .style         = CS_OWNDC|CS_HREDRAW|CS_VREDRAW,
        .lpfnWndProc   = Win32WindowProc,
        .hInstance     = Instance,
        .hCursor       = 0,
        .hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
        .lpszClassName = "Win32WindowClass",
    };
    
    if (RegisterClassA(&WindowClass))
    {
        HWND WindowHandle = CreateWindowA(WindowClass.lpszClassName,
                                          Params.WindowTitle,
                                          WS_OVERLAPPEDWINDOW,
                                          Params.WindowX, Params.WindowY,
                                          Params.WindowW, Params.WindowH,
                                          0, 0, Instance, 0);
        if (WindowHandle)
        {
            ShowWindow(WindowHandle, SW_SHOWNORMAL);

            app_input Input = { 0 };

            while (G_Running)
            {
                bool QuitRequested = false;

                MSG Message;
                while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
                {
                    switch (Message.message)
                    {
                        case WM_CLOSE:
                        case WM_QUIT:
                        {
                            QuitRequested = true;
                        } break;

                        default:
                        {
                            TranslateMessage(&Message);
                            DispatchMessageA(&Message);
                        } break;
                    }
                }

                Params.AppTick(API, &Input);

                if (QuitRequested)
                {
                    G_Running = false;
                }
            }
        }
    }

    for (win32_allocation_header *Header = G_Win32State.AllocationSentinel.Next;
         Header != &G_Win32State.AllocationSentinel;
         Header = Header->Next)
    {
        printf("Allocated Block, Size: %llu, Tag: %s\n", Header->Size, Header->Tag);
        if (!(Header->Flags & MemFlag_NoLeakCheck))
        {
            INVALID_CODE_PATH;
        }
    }
}
