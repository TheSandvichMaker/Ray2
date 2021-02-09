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
GLLoadTexture(int W, int H, void *Data)
{
    GLuint TextureHandle;
    glGenTextures(1, &TextureHandle);
    glBindTexture(GL_TEXTURE_2D, TextureHandle);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 OpenGL.DefaultInternalTextureFormat,
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

internal GLuint
GLCompileShader(const char *VertexShaderSource, const char *FragmentShaderSource)
{
    GLint Success;
    char InfoLog[1024];

    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
    glCompileShader(VertexShader);

    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Success);
    if (!Success)
    {
        glGetShaderInfoLog(VertexShader, sizeof(InfoLog), NULL, InfoLog);
        fprintf(stderr, "OpenGL Vertex Shader Compilation Failed: %s\n", InfoLog);
    }

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

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    return ShaderProgram;
}

internal GLuint
GLCompileBasicShader(void)
{
    const char *VertexShaderSource = 
        "#version 330 core\n"
        "layout (location = 0) in vec3 InVertexP;\n"
        "layout (location = 1) in vec4 InVertexColor;\n"
        "layout (location = 2) in vec2 InTexCoord;\n"
        "out vec4 VertexColor;\n"
        "out vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(InVertexP.xyz, 1.0f);\n"
        "    VertexColor = InVertexColor;\n"
        "    TexCoord = InTexCoord;\n"
        "}\n";

    const char *FragmentShaderSource = 
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec4 VertexColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D Texture;\n"
        "void main()\n"
        "{\n"
        "    FragColor = VertexColor*texture(Texture, TexCoord);\n"
        "}\n";

    return GLCompileShader(VertexShaderSource, FragmentShaderSource);
}

internal GLuint
GLCompileHdrBlitShader(void)
{
    const char *VertexShaderSource = 
        "#version 330 core\n"
        "layout (location = 0) in vec3 InVertexP;\n"
        "layout (location = 1) in vec4 InVertexColor;\n"
        "layout (location = 2) in vec2 InTexCoord;\n"
        "out vec4 VertexColor;\n"
        "out vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(InVertexP.xyz, 1.0f);\n"
        "    VertexColor = InVertexColor;\n"
        "    TexCoord = InTexCoord;\n"
        "}\n";

    const char *FragmentShaderSource = 
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec4 VertexColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D Texture;\n"
        "void main()\n"
        "{\n"
        "    FragColor = VertexColor*texture(Texture, TexCoord);\n"
        "    FragColor.rgb = vec3(1.0f) - exp(-FragColor.rgb);\n"
        "}\n";

    return GLCompileShader(VertexShaderSource, FragmentShaderSource);
}

typedef struct textured_vertex
{
    v3 P;
    v4 Color;
    v2 UV;
} textured_vertex;

internal void
GLFullscreenPass(void)
{
    textured_vertex Quad[] =
    {
        { .P = { -1, -1, 0 }, .Color = { 1, 1, 1, 1 }, .UV = { 0, 0 } },
        { .P = {  1, -1, 0 }, .Color = { 1, 1, 1, 1 }, .UV = { 1, 0 } },
        { .P = {  1,  1, 0 }, .Color = { 1, 1, 1, 1 }, .UV = { 1, 1 } },
        { .P = { -1, -1, 0 }, .Color = { 1, 1, 1, 1 }, .UV = { 0, 0 } },
        { .P = {  1,  1, 0 }, .Color = { 1, 1, 1, 1 }, .UV = { 1, 1 } },
        { .P = { -1,  1, 0 }, .Color = { 1, 1, 1, 1 }, .UV = { 0, 1 } },
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(Quad), Quad, GL_STREAM_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(textured_vertex), (void *)offsetof(textured_vertex, P));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(textured_vertex), (void *)offsetof(textured_vertex, Color));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(textured_vertex), (void *)offsetof(textured_vertex, UV));
    glEnableVertexAttribArray(2);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

internal void
GLDisplayBitmap(int W, int H, void *Pixels)
{
    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, OpenGL.DisplayImageTextureHandle);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 OpenGL.DefaultInternalTextureFormat,
                 W, H,
                 0,
                 GL_BGRA_EXT,
                 GL_UNSIGNED_BYTE,
                 Pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUseProgram(OpenGL.ShaderProgram);
    GLFullscreenPass();
}

internal void
GLDisplayHdrBuffer(int W, int H, void *Pixels)
{
    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, OpenGL.DisplayImageTextureHandle);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB32F,
                 W, H,
                 0,
                 GL_RGB,
                 GL_FLOAT,
                 Pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUseProgram(OpenGL.HdrBlit);
    GLFullscreenPass();
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
            case GL_DEBUG_SEVERITY_LOW    : { SeverityString = "Low";    } break;
            case GL_DEBUG_SEVERITY_MEDIUM : { SeverityString = "Medium"; } break;
            case GL_DEBUG_SEVERITY_HIGH   : { SeverityString = "High";   } break;
        }
        fprintf(stderr, "OpenGL Error (Severity: %s): %s\n", SeverityString, ErrorMessage);
        Assert(!"OpenGL Error Encountered!");
    }
}

internal void
GLInit(void)
{
    OpenGL.DefaultInternalTextureFormat = GL_SRGB8_ALPHA8;
    glEnable(GL_FRAMEBUFFER_SRGB);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &OpenGL.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, OpenGL.VBO);

    glGenTextures(1, &OpenGL.DisplayImageTextureHandle);

    OpenGL.ShaderProgram = GLCompileBasicShader();
    OpenGL.HdrBlit = GLCompileHdrBlitShader();

    if (glDebugMessageCallbackARB)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallbackARB(GLDebugCallback, 0);
    }
}
