#include "win32_ray.h"

#include <stdio.h>

global b32 G_Running = true;
global win32_state G_Win32State;
global LARGE_INTEGER G_PerfFreq;

//
// Platform Callbacks
//

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

#define GL_FUNCTIONS(_)                                   \
    _(GLubyte *, glGetStringi, GLenum name, GLuint index) \
    _(void, glDebugMessageCallbackARB, gl_debug_proc *Callback, const void *UserParam)

GL_FUNCTIONS(WGL_DECLARE_FUNCTION)

#include "ray_opengl.c"

#define WGL_FUNCTIONS(_)                                                                                                                                  \
    _(BOOL, wglChoosePixelFormatARB, HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats); \
    _(HGLRC, wglCreateContextAttribsARB, HDC hDC, HGLRC hShareContext, const int *attribList);                                                            \
    _(BOOL, wglSwapIntervalEXT, int interval);                                                                                                            \
    _(const char*, wglGetExtensionsStringEXT);

WGL_FUNCTIONS(WGL_DECLARE_FUNCTION)

#define WGL_EXTENSIONS(_) \
    _(WGL_EXT_framebuffer_sRGB)

typedef struct wgl_info {
    WGL_EXTENSIONS(GL_DECLARE_EXTENSION_STRUCT_MEMBER)
} wgl_info;

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
        int AttributeList[ArrayCount(AlwaysAvailableAttributes) + 2*NumberOfExtendedAttributes + 1] = { 0 };

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
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB|WGL_CONTEXT_DEBUG_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
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
        GLInit(OpenGLInfo);
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

    if ((Params.WindowX != CW_USEDEFAULT) ||
        (Params.WindowY != CW_USEDEFAULT) ||
        (Params.WindowW != CW_USEDEFAULT) ||
        (Params.WindowH != CW_USEDEFAULT))
    {
        RECT WindowRect = { Params.WindowX, Params.WindowY, Params.WindowW, Params.WindowH };
        AdjustWindowRectEx(&WindowRect, WS_OVERLAPPEDWINDOW, false, 0);
        
        Win32RectSpecs(WindowRect, &Params.WindowX, &Params.WindowY, &Params.WindowW, &Params.WindowH);
    }
    
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

    GLuint BlitHandle;
    glGenTextures(1, &BlitHandle);

    int TestW = 320;
    int TestH = 240;
    u32 *TestPixels = Win32Allocate(sizeof(u32)*TestW*TestH, MemFlag_NoLeakCheck, LOCATION_STRING());

    for (int Y = 0; Y < TestH; ++Y)
    for (int X = 0; X < TestW; ++X)
    {
        TestPixels[Y*TestW + X] = 0xFFFFFFFF; // (255 << 24)|(((X & 255) << 16)|((Y & 255) << 8));
    }

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

        RECT ClientRect;
        GetClientRect(WindowHandle, &ClientRect);

        int ClientW, ClientH;
        Win32RectSpecs(ClientRect, 0, 0, &ClientW, &ClientH);

        GLDisplayBitmap(TestW, TestH, TestPixels, ClientW, ClientH, BlitHandle);

        SwapBuffers(WindowDC);

        if (QuitRequested)
        {
            G_Running = false;
        }
    }

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
