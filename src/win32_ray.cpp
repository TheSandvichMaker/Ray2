#include "win32_ray.h"
#include "external/robustwin32io.cpp"

#include <stdio.h>

extern "C"
{
__declspec(dllexport) unsigned long NvOptimusEnablement        = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

global b32 G_Running = true;
global win32_state G_Win32State;
global LARGE_INTEGER G_PerfFreq;
global platform_render_settings G_RenderSettings;

//
// Platform Callbacks
//

internal void *
Win32Reserve(usize Size, u32 Flags, const char *Tag)
{
    usize PageSize = Platform.PageSize;
    usize TotalSize = PageSize + Size;

    win32_allocation_header *Header = (win32_allocation_header *)VirtualAlloc(0, TotalSize, MEM_RESERVE, PAGE_NOACCESS);
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
        win32_allocation_header *Header = (win32_allocation_header *)((char *)Pointer - Platform.PageSize);
        Header->Prev->Next = Header->Next;
        Header->Next->Prev = Header->Prev;
        VirtualFree(Header, 0, MEM_RELEASE);
    }
}

internal string_u8
Win32ReadEntireFile(arena *Arena, const char *FileName)
{
    string_u8 Result = {};

    LARGE_INTEGER FileSize;
    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            temporary_memory ResultTemp = BeginTemporaryMemory(Arena);
            Result.Data = PushArrayNoClear(Arena, FileSize.QuadPart + 1, u8);

            LONGLONG GotSize;
            if (win32_sync_read(FileHandle, 0, FileSize.QuadPart, Result.Data, &GotSize))
            {
                Result.Count = (usize)GotSize;
                Result.Data[GotSize] = 0;
                CommitTemporaryMemory(&ResultTemp);
            }
            else
            {
                ZeroStruct(&Result);
            }
        }

        CloseHandle(FileHandle);
    }

    return Result;
}

internal bool 
Win32WriteEntireFile(const char *FileName, string_u8 Data)
{
    bool Result = false;

    HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LONGLONG GotSize;
        if (win32_sync_write(FileHandle, 0, Data.Count, Data.Data, &GotSize))
        {
            if ((usize)GotSize == Data.Count)
            {
                Result = true;
            }
        }

        CloseHandle(FileHandle);
    }

    return Result;
}

internal platform_semaphore_handle
Win32CreateSemaphore(int InitialCount, int MaxCount)
{
    HANDLE Win32Handle = CreateSemaphoreA(nullptr, InitialCount, MaxCount, nullptr);

    platform_semaphore_handle Result =
    {
        .Opaque = (void *)Win32Handle,
    };

    return Result;
}

internal void
Win32WaitOnSemaphore(platform_semaphore_handle Handle)
{
    HANDLE Win32Handle = (HANDLE)Handle.Opaque;
    WaitForSingleObject(Win32Handle, INFINITE);
}

internal void
Win32ReleaseSemaphore(platform_semaphore_handle Handle, int Count, int *PreviousCount)
{
    HANDLE Win32Handle = (HANDLE)Handle.Opaque;
    LONG PreviousCountLong;
    ReleaseSemaphore(Win32Handle, Count, &PreviousCountLong);
    if (PreviousCount) *PreviousCount = (int)PreviousCountLong;
}

struct win32_thread_data
{
    platform_semaphore_handle Semaphore;
    platform_thread_proc Proc;
    void *UserData;
};

internal DWORD WINAPI
Win32ThreadProc(void *Param)
{
    win32_thread_data *Win32Data = (win32_thread_data *)Param;

    platform_semaphore_handle Semaphore = Win32Data->Semaphore;
    platform_thread_proc Proc = Win32Data->Proc;
    void *UserData = Win32Data->UserData;

    Proc(UserData, Semaphore);

    return 0;
}

internal platform_thread_handle
Win32CreateThread(platform_thread_proc Proc, void *UserData)
{
    local_persist platform_semaphore_handle ThreadStartSemaphore;
    if (!ThreadStartSemaphore.Opaque)
    {
        ThreadStartSemaphore = Win32CreateSemaphore(0, 1);
    }

    win32_thread_data Win32ThreadData =
    {
        .Proc      = Proc,
        .UserData  = UserData,
        .Semaphore = ThreadStartSemaphore,
    };

    HANDLE Win32Handle = CreateThread(nullptr, 0, Win32ThreadProc, &Win32ThreadData, 0, nullptr);
    platform_thread_handle Result =
    {
        .Opaque = (void *)Win32Handle,
    };

    Win32WaitOnSemaphore(Win32ThreadData.Semaphore);

    return Result;
}

//
// OpenGL
//

#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB               0x2093
#define WGL_CONTEXT_FLAGS_ARB                     0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB                 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB          0x20A9
#define WGL_SAMPLE_BUFFERS_ARB                    0x2041
#define WGL_SAMPLES_ARB                           0x2042
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023
#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B

#define WGL_DECLARE_FUNCTION(ReturnType, Name, ...)            \
    typedef ReturnType WINAPI gl_function_##Name(__VA_ARGS__); \
    global gl_function_##Name *Name;

#define WGL_LOAD_FUNCTION(ReturnType, Name, ...) \
    Name = (gl_function_##Name *)wglGetProcAddress(#Name);

#define GL_DEBUG_CALLBACK(Name) void WINAPI Name(GLenum Source, GLenum Type, GLuint Id, GLenum Severity, GLsizei Length, const GLchar *Message, const void *UserParam)
typedef GL_DEBUG_CALLBACK(gl_debug_proc);

GL_FUNCTIONS(WGL_DECLARE_FUNCTION)

#include "ray_opengl.cpp"

#define WGL_FUNCTIONS(_)                                                                            \
    _(BOOL, wglChoosePixelFormatARB, HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, \
                                     UINT nMaxFormats, int *piFormats, UINT *nNumFormats);          \
    _(HGLRC, wglCreateContextAttribsARB, HDC hDC, HGLRC hShareContext, const int *attribList);      \
    _(BOOL, wglSwapIntervalEXT, int interval);                                                      \
    _(const char*, wglGetExtensionsStringEXT);

WGL_FUNCTIONS(WGL_DECLARE_FUNCTION)

#define WGL_EXTENSIONS(_) \
    _(WGL_EXT_framebuffer_sRGB)

struct wgl_info
{
    WGL_EXTENSIONS(GL_DECLARE_EXTENSION_STRUCT_MEMBER)
};

internal void
WGLSetPixelFormat(HDC WindowDC, wgl_info *WGLInfo)
{
    int SuggestedPixelFormatIndex = 0;
    GLuint NumberOfExtendedFormatsFound = 0;

    if (wglChoosePixelFormatARB)
    {
        int AlwaysAvailableAttributes[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE                  ,
            WGL_ACCELERATION_ARB  , WGL_FULL_ACCELERATION_ARB,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE                  ,
            WGL_DOUBLE_BUFFER_ARB , GL_TRUE                  ,
            WGL_PIXEL_TYPE_ARB    , WGL_TYPE_RGBA_ARB        ,
            WGL_COLOR_BITS_ARB    , 32                       ,
            WGL_DEPTH_BITS_ARB    , 24                       ,
            WGL_STENCIL_BITS_ARB  , 8                        ,
        };

        const int NumberOfExtendedAttributes = 1;
        int AttributeList[ArrayCount(AlwaysAvailableAttributes) + 2*NumberOfExtendedAttributes + 1] = {};

        usize AttributeCursor = 0;
        for (; AttributeCursor < ArrayCount(AlwaysAvailableAttributes); ++AttributeCursor)
        {
            AttributeList[AttributeCursor] = AlwaysAvailableAttributes[AttributeCursor];
        }

        if (WGLInfo)
        {
            if (WGLInfo->WGL_EXT_framebuffer_sRGB)
            {
                AttributeList[AttributeCursor + 0] = WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB;
                AttributeList[AttributeCursor + 1] = GL_TRUE;
                AttributeCursor += 2;
            }
        }

        Assert((AttributeCursor < ArrayCount(AttributeList)) &&
               (AttributeList[AttributeCursor] == 0));

        wglChoosePixelFormatARB(WindowDC, AttributeList, 0, 1,
                                &SuggestedPixelFormatIndex, &NumberOfExtendedFormatsFound);
    }

    if (NumberOfExtendedFormatsFound == 0)
    {
        PIXELFORMATDESCRIPTOR DesiredPixelFormat =
        {
            .nVersion     = 1,
            .dwFlags      = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER,
            .iPixelType   = PFD_TYPE_RGBA,
            .cColorBits   = 32,
            .cAlphaBits   = 8,
            .cDepthBits   = 24,
            .cStencilBits = 8,
            .iLayerType   = PFD_MAIN_PLANE,
        };

        SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);
    }

    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex, sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);
}

internal bool
StringsAreEqual(usize Count, const char *A, const char *B)
{
    bool Result = true;
    for (usize CharIndex = 0; CharIndex < Count; ++CharIndex)
    {
        if (A[CharIndex] != B[CharIndex])
        {
            Result = false;
            break;
        }
    }
    return Result;
}

internal void
WGLLoadExtensionsAndFunctions(wgl_info *WGLInfo)
{
    WNDCLASSA WindowClass =
    {
        .lpfnWndProc = DefWindowProcA,
        .hInstance = 0,
        .lpszClassName = "WglLoaderWindow",
    };

    if (RegisterClass(&WindowClass))
    {
        HWND WindowHandle = CreateWindowA(WindowClass.lpszClassName,
                                          "WGL Loader",
                                          0,
                                          CW_USEDEFAULT, CW_USEDEFAULT,
                                          CW_USEDEFAULT, CW_USEDEFAULT,
                                          0, 0,
                                          WindowClass.hInstance,
                                          0);
        HDC WindowDC = GetDC(WindowHandle);

        WGLSetPixelFormat(WindowDC, 0);
        HGLRC GLRC = wglCreateContext(WindowDC);
        if (wglMakeCurrent(WindowDC, GLRC))
        {
            WGL_FUNCTIONS(WGL_LOAD_FUNCTION)

            char *Extensions = (char *)wglGetExtensionsStringEXT();
            char *At = Extensions;
            while (*At)
            {
                while (*At && (*At == ' ' || *At == '\t' || *At == '\r' || *At == '\n'))
                {
                    ++At;
                }

                char *End = At;

                while (*End && !(*End == ' ' || *End == '\t' || *End == '\r' || *End == '\n'))
                {
                    ++End;
                }

                usize Count = End - At;

#define GL_CHECK_EXTENSION(Name) if (StringsAreEqual(Count, At, #Name)) WGLInfo->Name = true;
                WGL_EXTENSIONS(GL_CHECK_EXTENSION)
#undef GL_CHECK_EXTENSION

                At = End;
            }

            wglMakeCurrent(0, 0);
        }

        wglDeleteContext(GLRC);
        ReleaseDC(WindowHandle, WindowDC);
        DestroyWindow(WindowHandle);
    }
}

internal HGLRC
WGLInit(HDC WindowDC, wgl_info *WGLInfo, opengl_info *OpenGLInfo)
{
    int WGLAttribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };

    WGLLoadExtensionsAndFunctions(WGLInfo);
    WGLSetPixelFormat(WindowDC, WGLInfo);
    HGLRC GLRC = wglCreateContextAttribsARB(WindowDC, 0, WGLAttribs);
    if (GLRC && wglMakeCurrent(WindowDC, GLRC))
    {
        if (wglSwapIntervalEXT)
        {
            wglSwapIntervalEXT(1);
        }

        GL_FUNCTIONS(WGL_LOAD_FUNCTION)
        GLGetInfo(OpenGLInfo);
        GLInit();
    }

    glFinish();

    return GLRC;
}

//
//
//

internal void
Win32RectSpecs(RECT Rect, int *X, int *Y, int *W, int *H)
{
    if (X) *X = Rect.left;
    if (Y) *Y = Rect.top;
    if (W) *W = Rect.right - Rect.left;
    if (H) *H = Rect.bottom - Rect.top;
}

internal LARGE_INTEGER
Win32GetClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

internal f64
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    f64 Result = (f64)(End.QuadPart - Start.QuadPart) / (f64)G_PerfFreq.QuadPart;
    return Result;
}

internal void
Win32ResizeImageBuffer(app_imagebuffer *ImageBuffer, u32 ClientW, u32 ClientH)
{
    if (ImageBuffer->Backbuffer)
    {
        Win32Deallocate(ImageBuffer->Backbuffer);
    }

    if (ImageBuffer->Frontbuffer)
    {
        Win32Deallocate(ImageBuffer->Frontbuffer);
    }
            
    ImageBuffer->W = ClientW;
    ImageBuffer->H = ClientH;
    if (ImageBuffer->W*ImageBuffer->H > 0)
    {
        ImageBuffer->Backbuffer = (app_pixel *)Win32Allocate(sizeof(app_pixel)*ImageBuffer->W*ImageBuffer->H,
                                                             0,
                                                             LOCATION_STRING("Win32 Backbuffer"));
        ImageBuffer->Frontbuffer = (app_pixel *)Win32Allocate(sizeof(app_pixel)*ImageBuffer->W*ImageBuffer->H,
                                                              0,
                                                              LOCATION_STRING("Win32 Frontbuffer"));
        fprintf(stderr, "Resized Image Buffer, W: %u, H: %u\n", ClientW, ClientH);
    }
}

internal void
Win32HandleKey(app_button *Button, bool EndedDown)
{
    Button->EndedDown = EndedDown;
    Button->HalfTransitionCount += 1;
}

internal void
Win32HandleKeyboardInput(app_input *Input, int VKCode, bool EndedDown)
{
    switch (VKCode)
    {
        case 'A': { Win32HandleKey(&Input->Buttons[AppButton_Left], EndedDown); } break;
        case 'D': { Win32HandleKey(&Input->Buttons[AppButton_Right], EndedDown); } break;
        case 'W': { Win32HandleKey(&Input->Buttons[AppButton_Forward], EndedDown); } break;
        case 'S': { Win32HandleKey(&Input->Buttons[AppButton_Back], EndedDown); } break;
        case VK_SPACE: { Win32HandleKey(&Input->Buttons[AppButton_Up], EndedDown); } break;
        case VK_CONTROL: { Win32HandleKey(&Input->Buttons[AppButton_Down], EndedDown); } break;
        case 'P':
        {
            G_RenderSettings.DEBUGShowBloomTexture += EndedDown;
        } break;
        case 'L':
        {
            G_RenderSettings.DEBUGShowBloomTexture -= EndedDown;
        } break;
    }

    if (Input->KeyEventCount < APP_KEY_EVENT_MAX)
    {
        app_key_event *Event = &Input->KeyEvents[Input->KeyEventCount++];
        Event->KeyCode = (platform_key_code)VKCode;
        Event->Pressed = EndedDown;
    }
}

internal void *
Win32GetTemporaryMemory(usize Size)
{
    local_persist usize BufferSize = 0;
    local_persist void *Buffer;

    Size = AlignPow2(Size, Platform.PageSize);

    if (BufferSize < Size)
    {
        BufferSize = Size;
        Win32Deallocate(Buffer);
        Buffer = Win32Allocate(Size, MemFlag_NoLeakCheck, LOCATION_STRING("Temporary Platform Memory"));
    }

    return Buffer;
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

#define ExitWithError(Error) do { MessageBoxA(0, Error, "Error", MB_OK); return -1; } while(0)

int
main(int ArgumentCount, char **Arguments)
{
    HINSTANCE Instance = 0;

    G_Win32State.AllocationSentinel.Next = &G_Win32State.AllocationSentinel;
    G_Win32State.AllocationSentinel.Prev = &G_Win32State.AllocationSentinel;

    QueryPerformanceFrequency(&G_PerfFreq);

    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);

    platform_api API =
    {
        .Reserve = Win32Reserve,
        .Commit = Win32Commit,
        .Allocate = Win32Allocate,
        .Deallocate = Win32Deallocate,
        .WriteEntireFile = Win32WriteEntireFile,
        .ReadEntireFile = Win32ReadEntireFile,
        .PageSize = SystemInfo.dwPageSize,
        .CreateThread = Win32CreateThread,
        .CreateSemaphore = Win32CreateSemaphore,
        .WaitOnSemaphore = Win32WaitOnSemaphore,
        .ReleaseSemaphore = Win32ReleaseSemaphore,
        .LogicalCoreCount = SystemInfo.dwNumberOfProcessors,
    };
    Platform = API;

    app_links Links = AppLinks();

    app_init_params Params =
    {
        .WindowTitle = "Unnamed Window",
        .WindowX = CW_USEDEFAULT,
        .WindowY = CW_USEDEFAULT,
        .WindowW = CW_USEDEFAULT,
        .WindowH = CW_USEDEFAULT,
    };

    if (Links.AppInit)
    {
        Links.AppInit(&Params);
    }

    if ((Params.WindowW != CW_USEDEFAULT) &&
        (Params.WindowH != CW_USEDEFAULT))
    {
        RECT WindowRect = { 0, 0, Params.WindowW, Params.WindowH };
        AdjustWindowRectEx(&WindowRect, WS_OVERLAPPEDWINDOW, false, 0);
        
        Win32RectSpecs(WindowRect, 0, 0, &Params.WindowW, &Params.WindowH);
    }
    
    HCURSOR ArrowCursor = LoadCursorA(nullptr, IDC_ARROW);
    WNDCLASSA WindowClass =
    {
        .style         = CS_OWNDC|CS_HREDRAW|CS_VREDRAW,
        .lpfnWndProc   = Win32WindowProc,
        .hInstance     = Instance,
        .hCursor       = 0,
        .hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
        .lpszClassName = "Win32WindowClass",
    };
    
    if (!RegisterClassA(&WindowClass))
    {
        ExitWithError("Could Not Register Window Class");
    }

    HWND WindowHandle = CreateWindowA(WindowClass.lpszClassName,
                                      Params.WindowTitle,
                                      WS_OVERLAPPEDWINDOW,
                                      Params.WindowX, Params.WindowY,
                                      Params.WindowW, Params.WindowH,
                                      0, 0, Instance, 0);

    if (!WindowHandle)
    {
        ExitWithError("Could Not Create Window");
    }

    HDC WindowDC = GetDC(WindowHandle);

    wgl_info WGLInfo;
    opengl_info OpenGLInfo;
    HGLRC GLRC = WGLInit(WindowDC, &WGLInfo, &OpenGLInfo);
    
    if (!GLRC)
    {
        ExitWithError("Could Not Initialize OpenGL");
    }

    fprintf(stderr, "Initialized OpenGL:\n");
    fprintf(stderr, "    Vendor                : %s\n", OpenGLInfo.Vendor);
    fprintf(stderr, "    Renderer              : %s\n", OpenGLInfo.Renderer);
    fprintf(stderr, "    Version               : %s\n", OpenGLInfo.Version);
    fprintf(stderr, "    ShadingLanguageVersion: %s\n", OpenGLInfo.ShadingLanguageVersion);
    fprintf(stderr, "\n");

    ShowWindow(WindowHandle, SW_SHOWNORMAL);

    int MonitorRefreshRate = GetDeviceCaps(WindowDC, VREFRESH);
    if (MonitorRefreshRate <= 1)
    {
        MonitorRefreshRate = 60;
    }

    RECT PrevClientRect = {};

    POINT PrevCursorP;
    GetCursorPos(&PrevCursorP);

    bool UsingRawInput = false;
    RAWINPUTDEVICE RawDevices[2] = {};
    
    // Mouse
    RawDevices[0].usUsagePage = 0x1;
    RawDevices[0].usUsage = 0x2;
    RawDevices[0].hwndTarget = WindowHandle;
    
    // Keyboard
    RawDevices[1].usUsagePage = 0x1;
    RawDevices[1].usUsage = 0x6;
    RawDevices[1].hwndTarget = WindowHandle;
    
    if (RegisterRawInputDevices(RawDevices, ArrayCount(RawDevices), sizeof(RAWINPUTDEVICE)))
    {
        UsingRawInput = true;
    }
    else
    {
        fprintf(stderr, "Failed to register raw input devices. Falling back to legacy message input.\n");
    }

    app_input Input = {};
    app_imagebuffer ImageBuffer = {};
    app_render_commands RenderCommands = {};
    RenderCommands.CommandBufferSize = Megabytes(1);
    RenderCommands.CommandBuffer = (char *)Win32Allocate(RenderCommands.CommandBufferSize, MemFlag_NoLeakCheck,
                                                         LOCATION_STRING("Win32 Render Commands"));

    LARGE_INTEGER StartClock = Win32GetClock();
    while (G_Running)
    {
        bool ExitRequested = false;

        for (usize I = 0; I < ArrayCount(Input.Buttons); ++I)
        {
            Input.Buttons[I].HalfTransitionCount = 0;
        }

        Input.FrameTime = 1.0f / (f32)MonitorRefreshRate;
        Input.MouseDeltaX = 0;
        Input.MouseDeltaY = 0;
        Input.RawMouseDeltaX = 0;
        Input.RawMouseDeltaY = 0;

        MSG Message;
        while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
        {
            switch (Message.message)
            {
                case WM_CLOSE:
                case WM_QUIT:
                {
                    ExitRequested = true;
                } break;

                case WM_INPUT:
                {
                    HRAWINPUT Handle = (HRAWINPUT)Message.lParam;
                    UINT Size;
                    GetRawInputData(Handle, RID_INPUT, 0, &Size, sizeof(RAWINPUTHEADER));
                    
                    BYTE *Data = (BYTE *)Win32GetTemporaryMemory(Size);
                    if (GetRawInputData(Handle, RID_INPUT, Data, &Size, sizeof(RAWINPUTHEADER)) != Size)
                    {
                        fprintf(stderr, "GetRawInputData returned an unexpected size.\n");
                    }
                    
                    RAWINPUT* Raw = (RAWINPUT *)Data;
                    if (Raw->header.dwType == RIM_TYPEMOUSE)
                    {
                        if (Raw->data.mouse.usButtonFlags)
                        {
                            if (Raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
                            {
                                Win32HandleKey(&Input.Buttons[AppButton_LeftMouse], true);
                            }
                            if (Raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
                            {
                                Win32HandleKey(&Input.Buttons[AppButton_LeftMouse], false);
                            }
                            if (Raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
                            {
                                Win32HandleKey(&Input.Buttons[AppButton_RightMouse], true);
                            }
                            if (Raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
                            {
                                Win32HandleKey(&Input.Buttons[AppButton_RightMouse], false);
                            }
                        }

                        if (Raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
                        {
                            fprintf(stderr, "Only siths deal in absolutes!\n");
                        }
                        else
                        {
                            Input.RawMouseDeltaX = Raw->data.mouse.lLastX;
                            Input.RawMouseDeltaY = Raw->data.mouse.lLastY;
                        }
                    }
                    else if (Raw->header.dwType == RIM_TYPEKEYBOARD)
                    {
                        bool IsDown = !(Raw->data.keyboard.Flags & RI_KEY_BREAK);
                        Win32HandleKeyboardInput(&Input, Raw->data.keyboard.VKey, IsDown);
                    }
                } break;

                case WM_LBUTTONDOWN:
                {
                    if (!UsingRawInput) Win32HandleKey(&Input.Buttons[AppButton_LeftMouse], true);
                } break;
                case WM_LBUTTONUP:
                {
                    if (!UsingRawInput) Win32HandleKey(&Input.Buttons[AppButton_LeftMouse], false);
                } break;
                case WM_RBUTTONDOWN:
                {
                    if (!UsingRawInput) Win32HandleKey(&Input.Buttons[AppButton_RightMouse], true);
                } break;
                case WM_RBUTTONUP:
                {
                    if (!UsingRawInput) Win32HandleKey(&Input.Buttons[AppButton_RightMouse], false);
                } break;

                case WM_KEYUP:
                case WM_KEYDOWN:
                case WM_SYSKEYUP:
                case WM_SYSKEYDOWN:
                {
                    if (!UsingRawInput)
                    {
                        u32 VKCode = (u32)Message.wParam;
                        b32 IsDown = ((Message.lParam & (1 << 31)) == 0);
                        Win32HandleKeyboardInput(&Input, VKCode, IsDown);
                    }
                } break;

                default:
                {
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                } break;
            }
        }

        RECT ClientRect;
        GetClientRect(WindowHandle, &ClientRect);

        int ClientW, ClientH;
        Win32RectSpecs(ClientRect, 0, 0, &ClientW, &ClientH);

        if (!StructsAreEqual(&ClientRect, &PrevClientRect))
        {
            Win32ResizeImageBuffer(&ImageBuffer, ClientW, ClientH);
            PrevClientRect = ClientRect;
        }

        POINT CursorP;
        GetCursorPos(&CursorP);

        POINT ClientCursorP = CursorP;
        ScreenToClient(WindowHandle, &ClientCursorP);

        Input.ClientMouseX = ClientCursorP.x;
        Input.ClientMouseY = ClientCursorP.y;
        Input.MouseDeltaX = CursorP.x - PrevCursorP.x;
        Input.MouseDeltaY = CursorP.y - PrevCursorP.y;

        RenderCommands.CommandBufferAt = 0;
        if (Links.AppTick)
        {
            Links.AppTick(API, &Input, &ImageBuffer, &RenderCommands);
            ExitRequested |= Input.ExitRequested;

            GLRenderCommands(&ImageBuffer, &G_RenderSettings, &RenderCommands);
        }

        if (Input.CaptureCursor)
        {
            POINT TopLeft = { 0, 0 };
            ClientToScreen(WindowHandle, &TopLeft);

            RECT ScreenspaceClientRect;
            ScreenspaceClientRect.left = TopLeft.x;
            ScreenspaceClientRect.top = TopLeft.y;
            ScreenspaceClientRect.right = TopLeft.x + ClientW;
            ScreenspaceClientRect.bottom = TopLeft.y + ClientH;

            POINT CursorResetP = { ScreenspaceClientRect.left + ClientW / 2, ScreenspaceClientRect.top + ClientH / 2 };
            ClipCursor(&ScreenspaceClientRect);
            SetCursor(nullptr);
            SetCursorPos(CursorResetP.x, CursorResetP.y);
            PrevCursorP = CursorResetP;
        }
        else
        {
            ClipCursor(nullptr);
            SetCursor(ArrowCursor);
            PrevCursorP = CursorP;
        }

        SwapBuffers(WindowDC);

        LARGE_INTEGER EndClock = Win32GetClock();
        f64 SecondsElapsed = Win32GetSecondsElapsed(StartClock, EndClock);
        StartClock = EndClock;

        char TitleBuffer[256];
        snprintf(TitleBuffer, sizeof(TitleBuffer), "Ray2 - %fms/f, %ffps",
                 1000.0*SecondsElapsed,
                 1.0 / SecondsElapsed);
        SetWindowTextA(WindowHandle, TitleBuffer);

        if (ExitRequested)
        {
            G_Running = false;
        }
    }

    if (Links.AppExit)
    {
        Links.AppExit();
    }

    Win32ResizeImageBuffer(&ImageBuffer, 0, 0);

    bool LeakedMemory = false;
    for (win32_allocation_header *Header = G_Win32State.AllocationSentinel.Next;
         Header != &G_Win32State.AllocationSentinel;
         Header = Header->Next)
    {
        printf("Allocated Block, Size: %llu, Tag: %s, NoLeakCheck: %s\n",
               Header->Size,
               Header->Tag,
               (Header->Flags & MemFlag_NoLeakCheck ? "true" : "false"));
        if (!(Header->Flags & MemFlag_NoLeakCheck))
        {
            LeakedMemory = true;
        }
    }

    if (LeakedMemory)
    {
        ExitWithError("Warning: Potentially Leaked Memory Detected");
    }

    return 0;
}
