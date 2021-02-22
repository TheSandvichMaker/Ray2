#ifndef RAY_RENDER_CONTEXT_H
#define RAY_RENDER_CONTEXT_H

struct render_context
{
    app_render_commands *Commands;
    u32 PushCommandFailureCount;
};

#endif /* RAY_RENDER_CONTEXT_H */
