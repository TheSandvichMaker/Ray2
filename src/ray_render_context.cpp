internal void
InitializeRenderContext(render_context *Context, app_render_commands *Commands)
{
    ZeroStruct(Context);
    Context->Commands = Commands;
}

internal void *
PushRenderCommand_(render_context *Context, usize Size, render_command_type Type)
{
    // NOTE: We're going to assume no render command is larger than 512 bytes
    Assert(Size < 512);
    local_persist char NullMemory[512];

    // NOTE: We return null memory so that if we fail to push a command, the calling code doesn't
    // need to care. It can just ignorantly write into this memory to no effect.
    void *Result = NullMemory; 
                                
    app_render_commands *Commands = Context->Commands;

    Assert(Commands->CommandBufferAt <= Commands->CommandBufferSize);
    usize SizeLeft = (Commands->CommandBufferSize - Commands->CommandBufferAt);
    usize SizeRequired = sizeof(render_command_header) + Size;
    if (SizeLeft >= SizeRequired)
    {
        render_command_header *Header = (render_command_header *)(Commands->CommandBuffer + Commands->CommandBufferAt);
        Commands->CommandBufferAt += SizeRequired;

        Header->Type = Type;
        Result = Header + 1;
    }
    else
    {
        ++Context->PushCommandFailureCount;
    }

    return Result;
}

#define PushRenderCommand(Context, Command) (render_command_##Command *)PushRenderCommand_(Context, sizeof(render_command_##Command), RenderCommand_##Command)

internal void
PushRect(render_context *Context, vec2 BottomLeft, vec2 WidthHeight, vec4 Color)
{
    render_command_rect *Command = PushRenderCommand(Context, rect);
    Command->Min = BottomLeft;
    Command->Max = BottomLeft + WidthHeight;
    Command->Color = Color;
}

internal void
PushClipRect(render_context *Context, vec2 BottomLeft, vec2 WidthHeight)
{
    render_command_clip_rect *Command = PushRenderCommand(Context, clip_rect);
    Command->Min = BottomLeft;
    Command->Max = BottomLeft + WidthHeight;
}
