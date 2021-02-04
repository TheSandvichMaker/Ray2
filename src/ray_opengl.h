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

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;

#define GL_DECLARE_EXTENSION_STRUCT_MEMBER(Extension) \
    b8 Extension;

#define MY_GL_EXTENSIONS(_)    \
    _(GL_EXT_texture_sRGB)     \
    _(GL_EXT_framebuffer_sRGB) \
    _(GL_ARB_framebuffer_sRGB)

typedef struct opengl_info {
    char *Vendor;
    char *Renderer;
    char *Version;
    char *ShadingLanguageVersion;
    char *Extensions;

    int MajorVersion;
    int MinorVersion;
    int BabyVersion;

    GLuint DefaultInternalTextureFormat;

    MY_GL_EXTENSIONS(GL_DECLARE_EXTENSION_STRUCT_MEMBER)
} opengl_info;

#endif /* RAY_OPENGL_H */
