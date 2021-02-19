#ifndef RAY_OPENGL_H
#define RAY_OPENGL_H

#define GL_FRAMEBUFFER                    0x8D40
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

#define GL_ACTIVE_UNIFORMS                0x8B86
#define GL_ACTIVE_UNIFORM_BLOCKS          0x8A36
#define GL_ACTIVE_ATTRIBUTES              0x8B89

#define GL_RGBA32F                        0x8814
#define GL_RGB32F                         0x8815

#define GL_CLAMP_TO_BORDER                0x812D
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_COLOR_ATTACHMENT2              0x8CE2
#define GL_COLOR_ATTACHMENT3              0x8CE3
#define GL_COLOR_ATTACHMENT4              0x8CE4
#define GL_COLOR_ATTACHMENT5              0x8CE5
#define GL_COLOR_ATTACHMENT6              0x8CE6
#define GL_COLOR_ATTACHMENT7              0x8CE7
#define GL_COLOR_ATTACHMENT8              0x8CE8
#define GL_COLOR_ATTACHMENT9              0x8CE9
#define GL_COLOR_ATTACHMENT10             0x8CEA
#define GL_COLOR_ATTACHMENT11             0x8CEB
#define GL_COLOR_ATTACHMENT12             0x8CEC
#define GL_COLOR_ATTACHMENT13             0x8CED
#define GL_COLOR_ATTACHMENT14             0x8CEE
#define GL_COLOR_ATTACHMENT15             0x8CEF
#define GL_COLOR_ATTACHMENT16             0x8CF0
#define GL_COLOR_ATTACHMENT17             0x8CF1
#define GL_COLOR_ATTACHMENT18             0x8CF2
#define GL_COLOR_ATTACHMENT19             0x8CF3
#define GL_COLOR_ATTACHMENT20             0x8CF4
#define GL_COLOR_ATTACHMENT21             0x8CF5
#define GL_COLOR_ATTACHMENT22             0x8CF6
#define GL_COLOR_ATTACHMENT23             0x8CF7
#define GL_COLOR_ATTACHMENT24             0x8CF8
#define GL_COLOR_ATTACHMENT25             0x8CF9
#define GL_COLOR_ATTACHMENT26             0x8CFA
#define GL_COLOR_ATTACHMENT27             0x8CFB
#define GL_COLOR_ATTACHMENT28             0x8CFC
#define GL_COLOR_ATTACHMENT29             0x8CFD
#define GL_COLOR_ATTACHMENT30             0x8CFE
#define GL_COLOR_ATTACHMENT31             0x8CFF

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef intptr_t GLintptr;
typedef int64_t GLint64;

#define GL_FUNCTIONS(_) \
    _(GLubyte *, glGetStringi, GLenum name, GLuint index) \
    _(void, glDebugMessageCallbackARB, gl_debug_proc *Callback, const void *UserParam) \
    _(void, glGenBuffers, GLsizei n, GLuint *buffers) \
    _(void, glBindBuffer, GLenum target, GLuint buffer) \
    _(void, glBufferData, GLenum target, GLsizeiptr size, const void *data, GLenum usage) \
    _(void, glAttachShader, GLuint program, GLuint shader) \
    _(void, glBindAttribLocation, GLuint program, GLuint index, const GLchar *name) \
    _(void, glCompileShader, GLuint shader) \
    _(GLuint, glCreateProgram, void) \
    _(GLuint, glCreateShader, GLenum type) \
    _(void, glDeleteProgram, GLuint program) \
    _(void, glDeleteShader, GLuint shader) \
    _(void, glDetachShader, GLuint program, GLuint shader) \
    _(void, glDrawBuffers, GLsizei n, const GLenum *bufs) \
    _(void, glGetShaderiv, GLuint shader, GLenum pname, GLint *params) \
    _(void, glGetShaderInfoLog, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
    _(void, glGetShaderSource, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) \
    _(GLint, glGetUniformLocation, GLuint program, const GLchar *name) \
    _(void, glGetUniformfv, GLuint program, GLint location, GLfloat *params) \
    _(void, glGetUniformiv, GLuint program, GLint location, GLint *params) \
    _(void, glGetVertexAttribdv, GLuint index, GLenum pname, GLdouble *params) \
    _(void, glGetVertexAttribfv, GLuint index, GLenum pname, GLfloat *params) \
    _(void, glGetVertexAttribiv, GLuint index, GLenum pname, GLint *params) \
    _(void, glGetVertexAttribPointerv, GLuint index, GLenum pname, void **pointer) \
    _(GLboolean, glIsProgram, GLuint program) \
    _(GLboolean, glIsShader, GLuint shader) \
    _(void, glLinkProgram, GLuint program) \
    _(void, glShaderSource, GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length) \
    _(void, glUseProgram, GLuint program) \
    _(void, glUniform1f, GLint location, GLfloat v0) \
    _(void, glUniform2f, GLint location, GLfloat v0, GLfloat v1) \
    _(void, glUniform3f, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) \
    _(void, glUniform4f, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) \
    _(void, glUniform1i, GLint location, GLint v0) \
    _(void, glUniform2i, GLint location, GLint v0, GLint v1) \
    _(void, glUniform3i, GLint location, GLint v0, GLint v1, GLint v2) \
    _(void, glUniform4i, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) \
    _(void, glUniform1fv, GLint location, GLsizei count, const GLfloat *value) \
    _(void, glUniform2fv, GLint location, GLsizei count, const GLfloat *value) \
    _(void, glUniform3fv, GLint location, GLsizei count, const GLfloat *value) \
    _(void, glUniform4fv, GLint location, GLsizei count, const GLfloat *value) \
    _(void, glUniform1iv, GLint location, GLsizei count, const GLint *value) \
    _(void, glUniform2iv, GLint location, GLsizei count, const GLint *value) \
    _(void, glUniform3iv, GLint location, GLsizei count, const GLint *value) \
    _(void, glUniform4iv, GLint location, GLsizei count, const GLint *value) \
    _(void, glUniformMatrix2fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    _(void, glUniformMatrix3fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    _(void, glUniformMatrix4fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    _(void, glValidateProgram, GLuint program) \
    _(void, glVertexAttrib1d, GLuint index, GLdouble x) \
    _(void, glVertexAttrib1dv, GLuint index, const GLdouble *v) \
    _(void, glVertexAttrib1f, GLuint index, GLfloat x) \
    _(void, glVertexAttrib1fv, GLuint index, const GLfloat *v) \
    _(void, glVertexAttrib1s, GLuint index, GLshort x) \
    _(void, glVertexAttrib1sv, GLuint index, const GLshort *v) \
    _(void, glVertexAttrib2d, GLuint index, GLdouble x, GLdouble y) \
    _(void, glVertexAttrib2dv, GLuint index, const GLdouble *v) \
    _(void, glVertexAttrib2f, GLuint index, GLfloat x, GLfloat y) \
    _(void, glVertexAttrib2fv, GLuint index, const GLfloat *v) \
    _(void, glVertexAttrib2s, GLuint index, GLshort x, GLshort y) \
    _(void, glVertexAttrib2sv, GLuint index, const GLshort *v) \
    _(void, glVertexAttrib3d, GLuint index, GLdouble x, GLdouble y, GLdouble z) \
    _(void, glVertexAttrib3dv, GLuint index, const GLdouble *v) \
    _(void, glVertexAttrib3f, GLuint index, GLfloat x, GLfloat y, GLfloat z) \
    _(void, glVertexAttrib3fv, GLuint index, const GLfloat *v) \
    _(void, glVertexAttrib3s, GLuint index, GLshort x, GLshort y, GLshort z) \
    _(void, glVertexAttrib3sv, GLuint index, const GLshort *v) \
    _(void, glVertexAttrib4Nbv, GLuint index, const GLbyte *v) \
    _(void, glVertexAttrib4Niv, GLuint index, const GLint *v) \
    _(void, glVertexAttrib4Nsv, GLuint index, const GLshort *v) \
    _(void, glVertexAttrib4Nub, GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w) \
    _(void, glVertexAttrib4Nubv, GLuint index, const GLubyte *v) \
    _(void, glVertexAttrib4Nuiv, GLuint index, const GLuint *v) \
    _(void, glVertexAttrib4Nusv, GLuint index, const GLushort *v) \
    _(void, glVertexAttrib4bv, GLuint index, const GLbyte *v) \
    _(void, glVertexAttrib4d, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) \
    _(void, glVertexAttrib4dv, GLuint index, const GLdouble *v) \
    _(void, glVertexAttrib4f, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) \
    _(void, glVertexAttrib4fv, GLuint index, const GLfloat *v) \
    _(void, glVertexAttrib4iv, GLuint index, const GLint *v) \
    _(void, glVertexAttrib4s, GLuint index, GLshort x, GLshort y, GLshort z, GLshort w) \
    _(void, glVertexAttrib4sv, GLuint index, const GLshort *v) \
    _(void, glVertexAttrib4ubv, GLuint index, const GLubyte *v) \
    _(void, glVertexAttrib4uiv, GLuint index, const GLuint *v) \
    _(void, glVertexAttrib4usv, GLuint index, const GLushort *v) \
    _(void, glVertexAttribPointer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) \
    _(GLint, glGetAttribLocation, GLuint program, const GLchar *name) \
    _(void, glGetActiveAttrib, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) \
    _(void, glGetActiveUniform, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) \
    _(void, glGetProgramiv, GLuint program, GLenum pname, GLint *params) \
    _(void, glGetProgramInfoLog, GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
    _(void, glDisableVertexAttribArray, GLuint index) \
    _(void, glEnableVertexAttribArray, GLuint index) \
    _(void, glBindVertexArray, GLuint array) \
    _(void, glDeleteVertexArrays, GLsizei n, const GLuint *arrays) \
    _(void, glGenVertexArrays, GLsizei n, GLuint *arrays) \
    _(void, glCreateTextures, GLenum target, GLsizei n, GLuint *textures) \
    _(void, glTextureParameterf, GLuint texture, GLenum pname, GLfloat param) \
    _(void, glTextureParameterfv, GLuint texture, GLenum pname, const GLfloat *param) \
    _(void, glTextureParameteri, GLuint texture, GLenum pname, GLint param) \
    _(void, glTextureParameterIiv, GLuint texture, GLenum pname, const GLint *params) \
    _(void, glTextureParameterIuiv, GLuint texture, GLenum pname, const GLuint *params) \
    _(void, glTextureParameteriv, GLuint texture, GLenum pname, const GLint *param) \
    _(void, glTextureStorage1D, GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width) \
    _(void, glTextureStorage2D, GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) \
    _(void, glTextureStorage3D, GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) \
    _(void, glTextureStorage2DMultisample, GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) \
    _(void, glTextureStorage3DMultisample, GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) \
    _(void, glTextureSubImage1D, GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels) \
    _(void, glTextureSubImage2D, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) \
    _(void, glTextureSubImage3D, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels) \
    _(void, glGenerateTextureMipmap, GLuint texture) \
    _(void, glBindTextureUnit, GLuint unit, GLuint texture) \
    _(void, glGetTextureImage, GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels) \
    _(void, glCreateFramebuffers, GLsizei n, GLuint *framebuffers) \
    _(void, glNamedFramebufferRenderbuffer, GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) \
    _(void, glNamedFramebufferParameteri, GLuint framebuffer, GLenum pname, GLint param) \
    _(void, glNamedFramebufferTexture, GLuint framebuffer, GLenum attachment, GLuint texture, GLint level) \
    _(void, glNamedFramebufferTextureLayer, GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer) \
    _(void, glNamedFramebufferDrawBuffer, GLuint framebuffer, GLenum buf) \
    _(void, glNamedFramebufferDrawBuffers, GLuint framebuffer, GLsizei n, const GLenum *bufs) \
    _(void, glNamedFramebufferReadBuffer, GLuint framebuffer, GLenum src) \
    _(void, glInvalidateNamedFramebufferSubData, GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height) \
    _(void, glClearNamedFramebufferiv, GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value) \
    _(void, glClearNamedFramebufferuiv, GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value) \
    _(void, glClearNamedFramebufferfv, GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value) \
    _(void, glClearNamedFramebufferfi, GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) \
    _(void, glBlitNamedFramebuffer, GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) \
    _(GLenum, glCheckNamedFramebufferStatus, GLuint framebuffer, GLenum target) \
    _(void, glGetNamedFramebufferParameteriv, GLuint framebuffer, GLenum pname, GLint *param) \
    _(void, glGetNamedFramebufferAttachmentParameteriv, GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params) \
    _(void, glCreateBuffers, GLsizei n, GLuint *buffers) \
    _(void, glNamedBufferStorage, GLuint buffer, GLsizeiptr size, const void *data, GLbitfield flags) \
    _(void, glNamedBufferData, GLuint buffer, GLsizeiptr size, const void *data, GLenum usage) \
    _(void, glNamedBufferSubData, GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data) \
    _(void, glCopyNamedBufferSubData, GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) \
    _(void, glClearNamedBufferData, GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void *data) \
    _(void, glClearNamedBufferSubData, GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data) \
    _(void *, glMapNamedBuffer, GLuint buffer, GLenum access) \
    _(void *, glMapNamedBufferRange, GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access) \
    _(GLboolean, glUnmapNamedBuffer, GLuint buffer) \
    _(void, glFlushMappedNamedBufferRange, GLuint buffer, GLintptr offset, GLsizeiptr length) \
    _(void, glGetNamedBufferParameteriv, GLuint buffer, GLenum pname, GLint *params) \
    _(void, glGetNamedBufferParameteri64v, GLuint buffer, GLenum pname, GLint64 *params) \
    _(void, glGetNamedBufferPointerv, GLuint buffer, GLenum pname, void **params) \
    _(void, glGetNamedBufferSubData, GLuint buffer, GLintptr offset, GLsizeiptr size, void *data) \
    _(void, glBindVertexBuffer, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) \
    _(void, glVertexAttribFormat, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) \
    _(void, glBindFramebuffer, GLenum target, GLuint framebuffer) \
    _(void, glDeleteFramebuffers, GLsizei n, const GLuint *framebuffers)

#define GL_DECLARE_EXTENSION_STRUCT_MEMBER(Extension) \
    b8 Extension;

#define MY_GL_EXTENSIONS(_)    \
    _(GL_EXT_texture_sRGB)     \
    _(GL_EXT_framebuffer_sRGB) \
    _(GL_ARB_framebuffer_sRGB)

struct opengl_info
{
    char *Vendor;
    char *Renderer;
    char *Version;
    char *ShadingLanguageVersion;

    int MajorVersion;
    int MinorVersion;
    int BabyVersion;

    MY_GL_EXTENSIONS(GL_DECLARE_EXTENSION_STRUCT_MEMBER)
};

struct opengl_program_common
{
    GLuint Program;
    GLuint FrameIndex;
};

struct opengl_bloom_blur_program
{
    opengl_program_common Common;
};

struct opengl_bloom_downsample_program
{
    opengl_program_common Common;
};

struct opengl_scale_hdr_program
{
    opengl_program_common Common;
};

struct opengl_hdr_blit_program
{
    opengl_program_common Common;
    GLuint SourceTexture;
    GLuint BloomTexture0;
    GLuint BloomTexture1;
    GLuint BloomTexture2;
    GLuint BloomTexture3;
    GLuint BloomTexture4;
    GLuint BloomTexture5;
    GLuint BloomTexture6;
    GLuint BloomTexture7;
};

struct opengl_framebuffer
{
    int W, H;
    GLuint FramebufferHandle;
    GLuint TextureHandle;
};

struct opengl_state
{
    platform_render_settings Settings;

    int FrameIndex;

    GLuint VBO;

    opengl_scale_hdr_program ScaleHdrProgram;
    opengl_hdr_blit_program HdrBlit;
    opengl_bloom_downsample_program BloomDownsampleProgram;
    opengl_bloom_blur_program BloomBlurProgram;

    GLuint DefaultInternalTextureFormat;
    GLuint WhiteTexture;

    u32 DisplayTextureW, DisplayTextureH;
    GLuint DisplayImageTextureHandle;

    opengl_framebuffer Backbuffer;

    int BloomFramebufferCount;
    opengl_framebuffer BloomFramebuffers[8];
    opengl_framebuffer BloomPongFramebuffers[8];
};

global opengl_state OpenGL;

#endif /* RAY_OPENGL_H */
