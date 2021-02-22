#ifndef RAY_RENDER_COMMANDS_H
#define RAY_RENDER_COMMANDS_H

enum render_command_type
{
    RenderCommand_rect,
    RenderCommand_clip_rect,
};

struct render_command_header
{
    render_command_type Type;
};

struct render_command_rect
{
    vec2 Min;
    vec2 Max;
    vec4 Color;
};

struct render_command_clip_rect
{
    vec2 Min;
    vec2 Max;
};

#endif /* RAY_RENDER_COMMANDS_H */
