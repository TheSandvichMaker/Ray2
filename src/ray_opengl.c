internal bool
GLCheckString(const char *A, const char *B)
{
    bool Result = true;
    while (*A && *B)
    {
        if (*A != *B)
        {
            Result = false;
            break;
        }
        ++A;
        ++B;
    }
    return Result;
}

internal void
GLCheckExtension(opengl_info *Info, const char *Extension)
{
#define GL_CHECK_EXTENSION(TestExtension) if (GLCheckString(Extension, #TestExtension)) Info->TestExtension = true;
    MY_GL_EXTENSIONS(GL_CHECK_EXTENSION)
#undef GL_CHECK_EXTENSION
}

internal void
GLGetInfo(opengl_info *Info)
{
    Info->Vendor                 = (char *)glGetString(GL_VENDOR);
    Info->Renderer               = (char *)glGetString(GL_RENDERER);
    Info->Version                = (char *)glGetString(GL_VERSION);
    Info->ShadingLanguageVersion = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    Info->Extensions             = (char *)glGetString(GL_EXTENSIONS);

    GLint ExtensionCount;
    glGetIntegerv(GL_NUM_EXTENSIONS, &ExtensionCount);
    for (GLint ExtensionIndex = 0; ExtensionIndex < ExtensionCount; ++ExtensionIndex)
    {
        char *Extension = (char *)glGetStringi(GL_EXTENSIONS, ExtensionIndex);
        GLCheckExtension(Info, Extension);
    }
}

internal void
GLSetScreenspace(int W, int H)
{
    Assert(W > 0 && H > 0);
    glMatrixMode(GL_PROJECTION);
    float A = 2.0f / (float)W;
    float B = 2.0f / (float)H;
    float ProjectionMatrix[] =
    {
         A,  0,  0,  0,
         0,  B,  0,  0,
         0,  0,  1,  0,
        -1, -1,  0,  1,
    };
    glLoadMatrixf(ProjectionMatrix);
}

internal GLuint
GLLoadTexture(opengl_info *Info, int W, int H, void *Data)
{
    GLuint TextureHandle;
    glGenTextures(1, &TextureHandle);
    glBindTexture(GL_TEXTURE_2D, TextureHandle);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 Info->DefaultInternalTextureFormat,
                 W, H,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 Data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();
    return TextureHandle;
}

internal void
GLUnloadTexture(GLuint Handle)
{
    glDeleteTextures(1, &Handle);
}

internal void
GLDisplayBitmap(int W, int H, void *Data, int WindowW, int WindowH, GLuint BlitTextureHandle)
{
    glViewport(0, 0, WindowW, WindowH);
    glScissor(0, 0, WindowW, WindowH);

    glBindTexture(GL_TEXTURE_2D, BlitTextureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, W, H, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, Data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GLSetScreenspace(W, H);

    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glBegin(GL_TRIANGLES);
    glColor4f(1, 1, 1, 1);

    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    glTexCoord2f(1, 0);
    glVertex2f(1, 0);
    glTexCoord2f(1, 1);
    glVertex2f(1, 1);

    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    glTexCoord2f(1, 1);
    glVertex2f(1, 1);
    glTexCoord2f(0, 1);
    glVertex2f(0, 1);

    glEnd();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
}

internal
GL_DEBUG_CALLBACK(GLDebugCallback)
{
    char *ErrorMessage = (char *)Message;
    if (Severity >= GL_DEBUG_SEVERITY_LOW)
    {
        Assert(!"OpenGL Error Encountered!");
    }
}

internal void
GLInit(opengl_info *Info)
{
    Info->DefaultInternalTextureFormat = GL_SRGB8_ALPHA8;
    glEnable(GL_FRAMEBUFFER_SRGB);

    if (glDebugMessageCallbackARB)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallbackARB(GLDebugCallback, 0);
    }
}
