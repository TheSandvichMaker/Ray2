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

    GLSetScreenspace(WindowW, WindowH);

    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glBegin(GL_TRIANGLES);
    glColor4f(1, 1, 1, 1);

    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    glTexCoord2f(1, 0);
    glVertex2f(W, 0);
    glTexCoord2f(1, 1);
    glVertex2f(W, H);

    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    glTexCoord2f(1, 1);
    glVertex2f(W, H);
    glTexCoord2f(0, 1);
    glVertex2f(0, H);

    glEnd();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
}

internal void
GLCompileShaders(void)
{
    GLint Success;
    char InfoLog[1024];

    const char *VertexShaderSource = 
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\n";

    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
    glCompileShader(VertexShader);

    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Success);
    if (!Success)
    {
        glGetShaderInfoLog(VertexShader, sizeof(InfoLog), NULL, InfoLog);
        fprintf(stderr, "OpenGL Vertex Shader Compilation Failed: %s\n", InfoLog);
    }

    const char *FragmentShaderSource = 
        "#version 330 core\n"
        "out vec4 FragColor;"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(1.0f, 0.5f, 0.25f, 1.0f);\n"
        "}\n";

    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentShaderSource, NULL);
    glCompileShader(FragmentShader);

    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Success);
    if (!Success)
    {
        glGetShaderInfoLog(FragmentShader, sizeof(InfoLog), NULL, InfoLog);
        fprintf(stderr, "OpenGL Fragment Shader Compilation Failed: %s\n", InfoLog);
    }

    GLuint ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, VertexShader);
    glAttachShader(ShaderProgram, FragmentShader);
    glLinkProgram(ShaderProgram);
    
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (!Success)
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(InfoLog), NULL, InfoLog);
        fprintf(stderr, "OpenGL Program Link Error: %s\n", InfoLog);
    }

    OpenGL.ShaderProgram = ShaderProgram;

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
}

global float Triangles[] =
{
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.5f,  0.5f, 0.0f,
};

internal void
GLTestTriangle(void)
{
    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(OpenGL.ShaderProgram);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

internal
GL_DEBUG_CALLBACK(GLDebugCallback)
{
    char *ErrorMessage = (char *)Message;
    if ((Severity == GL_DEBUG_SEVERITY_LOW)    ||
        (Severity == GL_DEBUG_SEVERITY_MEDIUM) ||
        (Severity == GL_DEBUG_SEVERITY_HIGH))
    {
        char *SeverityString = "Unexpected Severity";
        switch (Severity)
        {
            case GL_DEBUG_SEVERITY_LOW   : { SeverityString = "Low"; } break;
            case GL_DEBUG_SEVERITY_MEDIUM: { SeverityString = "Medium"; } break;
            case GL_DEBUG_SEVERITY_HIGH  : { SeverityString = "High"; } break;
        }
        fprintf(stderr, "OpenGL Error (Severity: %s): %s\n", SeverityString, ErrorMessage);
        Assert(!"OpenGL Error Encountered!");
    }
}

internal void
GLInit(opengl_info *Info)
{
    Info->DefaultInternalTextureFormat = GL_SRGB8_ALPHA8;
    glEnable(GL_FRAMEBUFFER_SRGB);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &OpenGL.VBO);

    glBindBuffer(GL_ARRAY_BUFFER, OpenGL.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Triangles), Triangles, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    GLCompileShaders();

    if (glDebugMessageCallbackARB)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallbackARB(GLDebugCallback, 0);
    }
}
