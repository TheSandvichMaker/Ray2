#ifndef RAY_RENDER_COMMANDS_H
#define RAY_RENDER_COMMANDS_H

enum render_command_type
{
    RenderCommand_Rect,
    RenderCommand_ClipRect,
};

struct render_command
{
    render_command_type Type;

    vec2 Min;
    vec2 Max;
    vec4 Color;
};

#endif /* RAY_RENDER_COMMANDS_H */
