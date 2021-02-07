#include "win32_ray.h"

#include <stdio.h>

__declspec(dllexport) unsigned long NvOptimusEnablement        = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

global b32 G_Running = true;
global win32_state G_Win32State;
global LARGE_INTEGER G_PerfFreq;

//
// Platform Callbacks
//

internal void *
Win32Reserve(usize Size, u32 Flags, const char *Tag)
{
    usize PageSize = Platform.PageSize;
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
        win32_allocation_header *Header = (win32_allocation_header *)((char *)Pointer - Platform.PageSize);
        Header->Prev->Next = Header->Next;
        Header->Next->Prev = Header->Prev;
        VirtualFree(Header, 0, MEM_RELEASE);
    }
}

internal platform_semaphore_handle
Win32CreateSemaphore(int InitialCount, int MaxCount)
{
    HANDLE Win32Handle = CreateSemaphoreA(NULL, InitialCount, MaxCount, NULL);

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

typedef struct win32_thread_data
{
    platform_semaphore_handle Semaphore;
    platform_thread_proc Proc;
    void *UserData;
} win32_thread_data;

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

    HANDLE Win32Handle = CreateThread(NULL, 0, Win32ThreadProc, &Win32ThreadData, 0, NULL);
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

#define GL_FUNCTIONS(_)                                                                                    \
    _(GLubyte *, glGetStringi, GLenum name, GLuint index)                                                  \
    _(void, glDebugMessageCallbackARB, gl_debug_proc *Callback, const void *UserParam)                     \
    _(void, glGenBuffers, GLsizei n, GLuint *buffers)                                                      \
    _(void, glBindBuffer, GLenum target, GLuint buffer)                                                    \
    _(void, glBufferData, GLenum target, GLsizeiptr size, const void *data, GLenum usage)                  \
    _(void, glAttachShader, GLuint program, GLuint shader)                                                 \
    _(void, glBindAttribLocation, GLuint program, GLuint index, const GLchar *name)                        \
    _(void, glCompileShader, GLuint shader)                                                                \
    _(GLuint, glCreateProgram, void)                                                                       \
    _(GLuint, glCreateShader, GLenum type)                                                                 \
    _(void, glDeleteProgram, GLuint program)                                                               \
    _(void, glDeleteShader, GLuint shader)                                                                 \
    _(void, glDetachShader, GLuint program, GLuint shader)                                                 \
    _(void, glDrawBuffers, GLsizei n, const GLenum *bufs)                                                  \
    _(void, glGetShaderiv, GLuint shader, GLenum pname, GLint *params)                                     \
    _(void, glGetShaderInfoLog, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)          \
    _(void, glGetShaderSource, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)            \
    _(GLint, glGetUniformLocation, GLuint program, const GLchar *name)                                     \
    _(void, glGetUniformfv, GLuint program, GLint location, GLfloat *params)                               \
    _(void, glGetUniformiv, GLuint program, GLint location, GLint *params)                                 \
    _(void, glGetVertexAttribdv, GLuint index, GLenum pname, GLdouble *params)                             \
    _(void, glGetVertexAttribfv, GLuint index, GLenum pname, GLfloat *params)                              \
    _(void, glGetVertexAttribiv, GLuint index, GLenum pname, GLint *params)                                \
    _(void, glGetVertexAttribPointerv, GLuint index, GLenum pname, void **pointer)                         \
    _(GLboolean, glIsProgram, GLuint program)                                                              \
    _(GLboolean, glIsShader, GLuint shader)                                                                \
    _(void, glLinkProgram, GLuint program)                                                                 \
    _(void, glShaderSource, GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length) \
    _(void, glUseProgram, GLuint program)                                                                  \
    _(void, glUniform1f, GLint location, GLfloat v0)                                                       \
    _(void, glUniform2f, GLint location, GLfloat v0, GLfloat v1)                                           \
    _(void, glUniform3f, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)                               \
    _(void, glUniform4f, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)                   \
    _(void, glUniform1i, GLint location, GLint v0)                                                         \
    _(void, glUniform2i, GLint location, GLint v0, GLint v1)                                               \
    _(void, glUniform3i, GLint location, GLint v0, GLint v1, GLint v2)                                     \
    _(void, glUniform4i, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)                           \
    _(void, glUniform1fv, GLint location, GLsizei count, const GLfloat *value)                             \
    _(void, glUniform2fv, GLint location, GLsizei count, const GLfloat *value)                             \
    _(void, glUniform3fv, GLint location, GLsizei count, const GLfloat *value)                             \
    _(void, glUniform4fv, GLint location, GLsizei count, const GLfloat *value)                             \
    _(void, glUniform1iv, GLint location, GLsizei count, const GLint *value)                               \
    _(void, glUniform2iv, GLint location, GLsizei count, const GLint *value)                               \
    _(void, glUniform3iv, GLint location, GLsizei count, const GLint *value)                               \
    _(void, glUniform4iv, GLint location, GLsizei count, const GLint *value)                               \
    _(void, glUniformMatrix2fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)  \
    _(void, glUniformMatrix3fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)  \
    _(void, glUniformMatrix4fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)  \
    _(void, glValidateProgram, GLuint program)                                                             \
    _(void, glVertexAttrib1d, GLuint index, GLdouble x)                                                    \
    _(void, glVertexAttrib1dv, GLuint index, const GLdouble *v)                                            \
    _(void, glVertexAttrib1f, GLuint index, GLfloat x)                                                     \
    _(void, glVertexAttrib1fv, GLuint index, const GLfloat *v)                                             \
    _(void, glVertexAttrib1s, GLuint index, GLshort x)                                                     \
    _(void, glVertexAttrib1sv, GLuint index, const GLshort *v)                                             \
    _(void, glVertexAttrib2d, GLuint index, GLdouble x, GLdouble y)                                        \
    _(void, glVertexAttrib2dv, GLuint index, const GLdouble *v)                                            \
    _(void, glVertexAttrib2f, GLuint index, GLfloat x, GLfloat y)                                          \
    _(void, glVertexAttrib2fv, GLuint index, const GLfloat *v)                                             \
    _(void, glVertexAttrib2s, GLuint index, GLshort x, GLshort y)                                          \
    _(void, glVertexAttrib2sv, GLuint index, const GLshort *v)                                             \
    _(void, glVertexAttrib3d, GLuint index, GLdouble x, GLdouble y, GLdouble z)                            \
    _(void, glVertexAttrib3dv, GLuint index, const GLdouble *v)                                            \
    _(void, glVertexAttrib3f, GLuint index, GLfloat x, GLfloat y, GLfloat z)                               \
    _(void, glVertexAttrib3fv, GLuint index, const GLfloat *v)                                             \
    _(void, glVertexAttrib3s, GLuint index, GLshort x, GLshort y, GLshort z)                               \
    _(void, glVertexAttrib3sv, GLuint index, const GLshort *v)                                             \
    _(void, glVertexAttrib4Nbv, GLuint index, const GLbyte *v)                                             \
    _(void, glVertexAttrib4Niv, GLuint index, const GLint *v)                                              \
    _(void, glVertexAttrib4Nsv, GLuint index, const GLshort *v)                                            \
    _(void, glVertexAttrib4Nub, GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)                  \
    _(void, glVertexAttrib4Nubv, GLuint index, const GLubyte *v)                                           \
    _(void, glVertexAttrib4Nuiv, GLuint index, const GLuint *v)                                            \
    _(void, glVertexAttrib4Nusv, GLuint index, const GLushort *v)                                          \
    _(void, glVertexAttrib4bv, GLuint index, const GLbyte *v)                                              \
    _(void, glVertexAttrib4d, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)                \
    _(void, glVertexAttrib4dv, GLuint index, const GLdouble *v)                                            \
    _(void, glVertexAttrib4f, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)                    \
    _(void, glVertexAttrib4fv, GLuint index, const GLfloat *v)                                             \
    _(void, glVertexAttrib4iv, GLuint index, const GLint *v)                                               \
    _(void, glVertexAttrib4s, GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)                    \
    _(void, glVertexAttrib4sv, GLuint index, const GLshort *v)                                             \
    _(void, glVertexAttrib4ubv, GLuint index, const GLubyte *v)                                            \
    _(void, glVertexAttrib4uiv, GLuint index, const GLuint *v)                                             \
    _(void, glVertexAttrib4usv, GLuint index, const GLushort *v)                                           \
    _(void, glVertexAttribPointer, GLuint index, GLint size, GLenum type, GLboolean normalized,            \
                                   GLsizei stride, const void *pointer)                                    \
    _(GLint, glGetAttribLocation, GLuint program, const GLchar *name)                                      \
    _(void, glGetProgramiv, GLuint program, GLenum pname, GLint *params)                                   \
    _(void, glGetProgramInfoLog, GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)        \
    _(void, glDisableVertexAttribArray, GLuint index)                                                      \
    _(void, glEnableVertexAttribArray, GLuint index)                                                       \
    _(void, glBindVertexArray, GLuint array)                                                               \
    _(void, glDeleteVertexArrays, GLsizei n, const GLuint *arrays)                                         \
    _(void, glGenVertexArrays, GLsizei n, GLuint *arrays)

GL_FUNCTIONS(WGL_DECLARE_FUNCTION)

#include "ray_opengl.c"

#define WGL_FUNCTIONS(_)                                                                            \
    _(BOOL, wglChoosePixelFormatARB, HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, \
                                     UINT nMaxFormats, int *piFormats, UINT *nNumFormats);          \
    _(HGLRC, wglCreateContextAttribsARB, HDC hDC, HGLRC hShareContext, const int *attribList);      \
    _(BOOL, wglSwapIntervalEXT, int interval);                                                      \
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
        .Reserve          = Win32Reserve,
        .Commit           = Win32Commit,
        .Allocate         = Win32Allocate,
        .Deallocate       = Win32Deallocate,
        .PageSize         = SystemInfo.dwPageSize,
        .CreateThread     = Win32CreateThread,
        .CreateSemaphore  = Win32CreateSemaphore,
        .WaitOnSemaphore  = Win32WaitOnSemaphore,
        .ReleaseSemaphore = Win32ReleaseSemaphore,
        .LogicalCoreCount = 8,
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
    Links.AppInit(&Params);

    if ((Params.WindowW != CW_USEDEFAULT) &&
        (Params.WindowH != CW_USEDEFAULT))
    {
        RECT WindowRect = { 0, 0, Params.WindowW, Params.WindowH };
        AdjustWindowRectEx(&WindowRect, WS_OVERLAPPEDWINDOW, false, 0);
        
        Win32RectSpecs(WindowRect, 0, 0, &Params.WindowW, &Params.WindowH);
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

    fprintf(stderr, "Initialized OpenGL:\n");
    fprintf(stderr, "    Vendor                : %s\n", OpenGLInfo.Vendor);
    fprintf(stderr, "    Renderer              : %s\n", OpenGLInfo.Renderer);
    fprintf(stderr, "    Version               : %s\n", OpenGLInfo.Version);
    fprintf(stderr, "    ShadingLanguageVersion: %s\n", OpenGLInfo.ShadingLanguageVersion);
    fprintf(stderr, "\n");

    ShowWindow(WindowHandle, SW_SHOWNORMAL);

    app_input Input = { 0 };

    RECT PrevClientRect = { 0 };
    platform_backbuffer Backbuffer = { 0 };

    LARGE_INTEGER StartClock = Win32GetClock();

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

        RECT ClientRect;
        GetClientRect(WindowHandle, &ClientRect);

        int ClientW, ClientH;
        Win32RectSpecs(ClientRect, 0, 0, &ClientW, &ClientH);

        if (!StructsAreEqual(&ClientRect, &PrevClientRect))
        {
            if (Backbuffer.Pixels)
            {
                Win32Deallocate(Backbuffer.Pixels);
            }
            
            Backbuffer.W = ClientW;
            Backbuffer.H = ClientH;
            Backbuffer.Pixels = Win32Allocate(sizeof(*Backbuffer.Pixels)*Backbuffer.W*Backbuffer.H,
                                              0,
                                              LOCATION_STRING("Win32 Backbuffer"));
            PrevClientRect = ClientRect;
        }

        Links.AppTick(API, &Input, &Backbuffer);

        GLDisplayBitmap(Backbuffer.W, Backbuffer.H, Backbuffer.Pixels);

        SwapBuffers(WindowDC);

        LARGE_INTEGER EndClock = Win32GetClock();
        f64 SecondsElapsed = Win32GetSecondsElapsed(StartClock, EndClock);
        StartClock = EndClock;

        char TitleBuffer[256];
        snprintf(TitleBuffer, sizeof(TitleBuffer), "Ray2 - %fms/f, %ffps",
                 1000.0*SecondsElapsed,
                 1.0 / SecondsElapsed);
        SetWindowTextA(WindowHandle, TitleBuffer);

        if (QuitRequested)
        {
            G_Running = false;
        }
    }

    if (Backbuffer.Pixels)
    {
        Win32Deallocate(Backbuffer.Pixels);
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
