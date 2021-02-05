#ifndef RAY_OPENGL_H
#define RAY_OPENGL_H

#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_SRGB8_ALPHA8                   0x8C43
#define GL_NUM_EXTENSIONS                 0x821D
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C

#define GL_MULTISAMPLE_ARB                0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE_ARB   0x809E
#define GL_SAMPLE_ALPHA_TO_ONE_ARB        0x809F
#define GL_SAMPLE_COVERAGE_ARB            0x80A0

#define GL_CLAMP_TO_EDGE                  0x812F

#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242

#define GL_STREAM_DRAW                    0x88E0
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

#define GL_ARRAY_BUFFER                   0x8892

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;

#define GL_DECLARE_EXTENSION_STRUCT_MEMBER(Extension) \
    b8 Extension;

#define MY_GL_EXTENSIONS(_)    \
    _(GL_EXT_texture_sRGB)     \
    _(GL_EXT_framebuffer_sRGB) \
    _(GL_ARB_framebuffer_sRGB)

typedef struct opengl_info
{
    char *Vendor;
    char *Renderer;
    char *Version;
    char *ShadingLanguageVersion;

    int MajorVersion;
    int MinorVersion;
    int BabyVersion;

    MY_GL_EXTENSIONS(GL_DECLARE_EXTENSION_STRUCT_MEMBER)
} opengl_info;

typedef struct opengl_state
{
    GLuint VBO;
    GLuint ShaderProgram;

    GLuint DefaultInternalTextureFormat;
    GLuint DisplayImageTextureHandle;
} opengl_state;

#if 0
typedef struct opengl_program_interface
{
    void (*
};
#endif

global opengl_state OpenGL;

#endif /* RAY_OPENGL_H */
