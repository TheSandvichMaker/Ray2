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
    Info->Vendor = (char *)glGetString(GL_VENDOR);
    Info->Renderer = (char *)glGetString(GL_RENDERER);
    Info->Version = (char *)glGetString(GL_VERSION);
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
    mat4 ProjectionMatrix =
    {
         A, 0, 0, 0, 
         0, B, 0, 0, 
         0, 0, 1, 0, 
        -1,-1, 0, 1, 
    };
    glLoadMatrixf((float *)ProjectionMatrix.Elements);
}

internal GLuint
GLLoadTexture(int W, int H, void *Data)
{
    GLuint TextureHandle;
    glCreateTextures(GL_TEXTURE_2D, 1, &TextureHandle);
    glTextureParameteri(TextureHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(TextureHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(TextureHandle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(TextureHandle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureStorage2D(TextureHandle, 1, OpenGL.DefaultInternalTextureFormat, W, H);
    glTextureSubImage2D(TextureHandle, 0, 0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, Data);
    return TextureHandle;
}

internal GLuint
GLFramebufferTexture(int W, int H)
{
    GLuint TextureHandle;
    glCreateTextures(GL_TEXTURE_2D, 1, &TextureHandle);
    glTextureParameteri(TextureHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(TextureHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(TextureHandle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(TextureHandle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTextureStorage2D(TextureHandle, 1, GL_RGB32F, W, H);
    return TextureHandle;
}

internal void
GLUnloadTexture(GLuint Handle)
{
    glDeleteTextures(1, &Handle);
}

#define V_ATTRIB_P        0
#define V_ATTRIB_COLOR    1
#define V_ATTRIB_TEXCOORD 2

internal GLuint
GLCompileProgram(opengl_program_common *Result, const char *VertexShaderSource, const char *FragmentShaderSource)
{
    GLint Success;
    char InfoLog[1024];

    const char *ShaderPreamble =
        "#version 330 core" "\n";

    const char *VertexShaderSources[] =
    {
        ShaderPreamble,
        "#define V_ATTRIB_P " Stringize(V_ATTRIB_P) "\n",
        "#define V_ATTRIB_COLOR " Stringize(V_ATTRIB_COLOR) "\n",
        "#define V_ATTRIB_TEXCOORD " Stringize(V_ATTRIB_TEXCOORD) "\n",
        VertexShaderSource,
    };

    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, ArrayCount(VertexShaderSources), VertexShaderSources, NULL);
    glCompileShader(VertexShader);

    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Success);
    if (!Success)
    {
        glGetShaderInfoLog(VertexShader, sizeof(InfoLog), NULL, InfoLog);
        fprintf(stderr, "OpenGL Vertex Shader Compilation Failed: %s\n", InfoLog);
    }

    const char *FragmentShaderSources[] =
    {
        ShaderPreamble,
        FragmentShaderSource,
    };

    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, ArrayCount(FragmentShaderSources), FragmentShaderSources, NULL);
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

    Result->Program = ShaderProgram;
    Result->FrameIndex = glGetUniformLocation(ShaderProgram, "FrameIndex");

    return ShaderProgram;
}

internal void
GLCompileBasicProgram(opengl_program_common *Result)
{
    const char *VertexShaderSource = 
    R"GLSL(
        layout (location = V_ATTRIB_P) in vec3 InVertexP;
        layout (location = V_ATTRIB_COLOR) in vec4 InVertexColor;
        layout (location = V_ATTRIB_TEXCOORD) in vec2 InTexCoord;

        out vec4 VertexColor;
        out vec2 TexCoord;

        void main()
        {
            gl_Position = vec4(InVertexP.xyz, 1.0f);
            VertexColor = InVertexColor;
            TexCoord = InTexCoord;
        }
    )GLSL";

    const char *FragmentShaderSource = 
    R"GLSL(
        in vec4 VertexColor;
        in vec2 TexCoord;

        out vec4 FragColor;

        uniform sampler2D Texture;

        void main()
        {
            FragColor = VertexColor*texture(Texture, TexCoord);
        }
    )GLSL";

    GLCompileProgram(Result, VertexShaderSource, FragmentShaderSource);
}

internal void
GLCompileBloomDownsampleProgram(opengl_bloom_downsample_program *Result)
{
    const char *VertexShaderSource = 
    R"GLSL(
        layout (location = V_ATTRIB_P) in vec3 InVertexP;
        layout (location = V_ATTRIB_COLOR) in vec4 InVertexColor;
        layout (location = V_ATTRIB_TEXCOORD) in vec2 InTexCoord;

        out vec4 VertexColor;
        out vec2 TexCoord;

        void main()
        {
            gl_Position = vec4(InVertexP.xyz, 1.0f);
            VertexColor = InVertexColor;
            TexCoord = InTexCoord;
        }
    )GLSL";

    const char *FragmentShaderSource = 
    R"GLSL(
        in vec4 VertexColor;
        in vec2 TexCoord;

        out vec4 FragColor;

        uniform vec2 SourcePixSize;
        uniform sampler2D SourceTexture;

        void main()
        {

            const vec2 Offsets[9] = vec2[](vec2(-1.0f,  1.0f),
                                           vec2( 0.0f,  1.0f),
                                           vec2( 1.0f,  1.0f),
                                           vec2(-1.0f,  0.0f),
                                           vec2( 0.0f,  0.0f),
                                           vec2( 1.0f,  0.0f),
                                           vec2(-1.0f, -1.0f),
                                           vec2( 0.0f, -1.0f),
                                           vec2( 1.0f, -1.0f));

            const float Weights[9] = float[](0.0625f, 0.125f, 0.0625f,
                                             0.125f , 0.25f , 0.125f ,
                                             0.0625f, 0.125f, 0.0625f);

            FragColor = vec4(0);
            for (int I = 0; I < 9; ++I)
            {
                FragColor += Weights[I]*texture(SourceTexture, TexCoord + SourcePixSize*Offsets[I]);
            }
        }
    )GLSL";

    GLuint Prog = GLCompileProgram(&Result->Common, VertexShaderSource, FragmentShaderSource);

    Result->SourcePixSize = glGetUniformLocation(Prog, "SourcePixSize");
    Result->SourceTexture = glGetUniformLocation(Prog, "SourceTexture");
}

internal void
GLCompileHdrBlitProgram(opengl_hdr_blit_program *Result)
{
    const char *VertexShaderSource =
    R"GLSL(
        layout (location = V_ATTRIB_P) in vec3 InVertexP;
        layout (location = V_ATTRIB_COLOR) in vec4 InVertexColor;
        layout (location = V_ATTRIB_TEXCOORD) in vec2 InTexCoord;

        out vec4 VertexColor;
        out vec2 TexCoord;

        void main()
        {
            gl_Position = vec4(InVertexP.xyz, 1.0f);
            VertexColor = InVertexColor;
            TexCoord = InTexCoord;
        }
    )GLSL";

    const char *FragmentShaderSource = 
    R"GLSL(
        in vec4 VertexColor;
        in vec2 TexCoord;

        out vec4 FragColor;

        uniform sampler2D SourceTexture;
        uniform int FrameIndex;

        uniform sampler2D BloomTexture0;
        uniform sampler2D BloomTexture1;
        uniform sampler2D BloomTexture2;
        uniform sampler2D BloomTexture3;
        uniform sampler2D BloomTexture4;
        uniform sampler2D BloomTexture5;
        uniform sampler2D BloomTexture6;
        uniform sampler2D BloomTexture7;

        // SOURCE: https://www.shadertoy.com/view/XlXcW4
        const uint k = 1103515245U;  // GLIB C
        //const uint k = 134775813U;   // Delphi and Turbo Pascal
        //const uint k = 20170906U;    // Today's date (use three days ago's dateif you want a prime)
        //const uint k = 1664525U;     // Numerical Recipes

        vec3 Hash(uvec3 x)
        {
            x = ((x>>8U)^x.yzx)*k;
            x = ((x>>8U)^x.yzx)*k;
            x = ((x>>8U)^x.yzx)*k;
            
            return vec3(x)*(1.0/float(0xffffffffU));
        }

        void main()
        {
            FragColor = VertexColor*texture(SourceTexture, TexCoord);
            float HdrScale = 1.0f / FragColor.a;
            FragColor.rgb *= HdrScale;
            FragColor.rgb += (1.0f / 8.0f)*(HdrScale*texture(BloomTexture0, TexCoord).rgb +
                                            HdrScale*texture(BloomTexture1, TexCoord).rgb +
                                            HdrScale*texture(BloomTexture2, TexCoord).rgb +
                                            HdrScale*texture(BloomTexture3, TexCoord).rgb +
                                            HdrScale*texture(BloomTexture4, TexCoord).rgb +
                                            HdrScale*texture(BloomTexture5, TexCoord).rgb +
                                            HdrScale*texture(BloomTexture6, TexCoord).rgb +
                                            HdrScale*texture(BloomTexture7, TexCoord).rgb);
            FragColor.rgb = vec3(1.0f) - exp(-FragColor.rgb);

            vec3 PrevDitherNoise = Hash(uvec3(gl_FragCoord.xy, FrameIndex - 1));
            vec3 DitherNoise = Hash(uvec3(gl_FragCoord.xy, FrameIndex));

            FragColor.rgb = sqrt(FragColor.rgb);
            vec3 Dither = 1.0f / 255.0f*(DitherNoise - PrevDitherNoise);
            FragColor.rgb += Dither;
            FragColor.rgb *= FragColor.rgb;
        }
    )GLSL";

    GLCompileProgram(&Result->Common, VertexShaderSource, FragmentShaderSource);
}

struct textured_vertex
{
    vec3 P;
    vec4 Color;
    vec2 UV;
};

internal void
GLBindFramebuffer(GLuint Handle, int W, int H)
{
    glViewport(0, 0, W, H);
    glScissor(0, 0, W, H);
    glBindFramebuffer(GL_FRAMEBUFFER, Handle);
}

internal void
GLBeginFullscreenPass()
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
}

internal void
GLEndFullscreenPass(void)
{
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

internal void
GLUseProgram(opengl_program_common *Program)
{
    glVertexAttribPointer(V_ATTRIB_P, 3, GL_FLOAT, GL_FALSE,
                          sizeof(textured_vertex), (void *)offsetof(textured_vertex, P));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(V_ATTRIB_COLOR, 4, GL_FLOAT, GL_FALSE,
                          sizeof(textured_vertex), (void *)offsetof(textured_vertex, Color));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(V_ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
                          sizeof(textured_vertex), (void *)offsetof(textured_vertex, UV));
    glEnableVertexAttribArray(2);

    glUseProgram(Program->Program);
    glUniform1i(Program->FrameIndex, OpenGL.FrameIndex);
}

internal void
GLUseProgram(opengl_hdr_blit_program *Program)
{
    GLUseProgram(&Program->Common);
    glUniform1i(Program->SourceTexture, 0);
    glUniform1i(Program->BloomTexture0, 1);
    glUniform1i(Program->BloomTexture1, 2);
    glUniform1i(Program->BloomTexture2, 3);
    glUniform1i(Program->BloomTexture3, 4);
    glUniform1i(Program->BloomTexture4, 5);
    glUniform1i(Program->BloomTexture5, 6);
    glUniform1i(Program->BloomTexture6, 7);
    glUniform1i(Program->BloomTexture7, 8);
}

internal void
GLUseProgram(opengl_bloom_downsample_program *Program, int SourceW, int SourceH)
{
    GLUseProgram(&Program->Common);
    glUniform2f(Program->SourcePixSize, 1.0f / (f32)SourceW, 1.0f / (f32)SourceH);
    glUniform1i(Program->SourceTexture, 0);
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

    GLBindFramebuffer(0, W, H);

    GLBeginFullscreenPass();
    GLUseProgram(&OpenGL.ShaderProgram);
    GLEndFullscreenPass();
}

internal void
GLDownsampleBloom(GLuint InitSourceTexture, int InitSourceW, int InitSourceH)
{
    GLuint SourceTexture = InitSourceTexture;
    GLuint SourceW = InitSourceW;
    GLuint SourceH = InitSourceH;

    for (int I = 0; I < OpenGL.BloomFramebufferCount; ++I)
    {
        opengl_framebuffer *Dest = OpenGL.BloomFramebuffers + I;

        GLBindFramebuffer(Dest->FramebufferHandle, Dest->W, Dest->H);
        glBindTextureUnit(0, SourceTexture);

        GLBeginFullscreenPass();
        GLUseProgram(&OpenGL.BloomDownsampleProgram, SourceW, SourceH);
        GLEndFullscreenPass();

        SourceTexture = Dest->TextureHandle;
        SourceW = Dest->W;
        SourceH = Dest->H;
    }
}

internal void
GLDisplayHdrBuffer(app_imagebuffer *Buffer)
{
    OpenGL.FrameIndex += 1;

    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, OpenGL.DisplayImageTextureHandle);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA32F,
                 Buffer->W, Buffer->H,
                 0,
                 GL_RGBA,
                 GL_FLOAT,
                 Buffer->Frontbuffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    if (OpenGL.BloomFramebufferCount == 0)
    {
        int W = Buffer->W;
        int H = Buffer->H;
        OpenGL.BloomFramebufferCount = 8;
        for (int I = 0; I < OpenGL.BloomFramebufferCount; ++I)
        {
            opengl_framebuffer *Framebuffer = OpenGL.BloomFramebuffers + I;
            Framebuffer->W = W;
            Framebuffer->H = H;

            Framebuffer->TextureHandle = GLFramebufferTexture(W, H);

            glCreateFramebuffers(1, &Framebuffer->FramebufferHandle);
            glNamedFramebufferTexture(Framebuffer->FramebufferHandle,
                                      GL_COLOR_ATTACHMENT0,
                                      Framebuffer->TextureHandle,
                                      0);

            W = (W + 1) / 2;
            H = (H + 1) / 2;
            if (W < 1)
            {
                W = 1;
            }
            if (H < 1)
            {
                H = 1;
            }
        }
    }

    GLDownsampleBloom(OpenGL.DisplayImageTextureHandle, Buffer->W, Buffer->H);

    glBindTextureUnit(0, OpenGL.DisplayImageTextureHandle);
    for (int I = 0; I < OpenGL.BloomFramebufferCount; ++I)
    {
        glBindTextureUnit(1 + I, OpenGL.BloomFramebuffers[I].TextureHandle);
    }

#if 1
    GLBindFramebuffer(0, Buffer->W, Buffer->H);
    GLBeginFullscreenPass();
    GLUseProgram(&OpenGL.HdrBlit);
    GLEndFullscreenPass();
#else
    int BlitIndex = 1;
    glBlitNamedFramebuffer(OpenGL.BloomFramebuffers[BlitIndex].FramebufferHandle,
                           0,
                           0, 0, OpenGL.BloomFramebuffers[BlitIndex].W, OpenGL.BloomFramebuffers[BlitIndex].H,
                           0, 0, Buffer->W, Buffer->H, 
                           GL_COLOR_BUFFER_BIT,
                           GL_LINEAR);
#endif
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

    GLCompileBasicProgram(&OpenGL.ShaderProgram);
    GLCompileHdrBlitProgram(&OpenGL.HdrBlit);
    GLCompileBloomDownsampleProgram(&OpenGL.BloomDownsampleProgram);

    if (glDebugMessageCallbackARB)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallbackARB(GLDebugCallback, 0);
    }
}
