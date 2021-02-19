internal void
InitializeRenderContext(render_context *Context, app_render_commands *Commands)
{
    ZeroStruct(Context);
    Context->Commands = Commands;
}

internal void
PushRect(render_context *Context, vec2 BottomLeft, vec2 WidthHeight, vec4 Color)
{
    app_render_commands *Commands = Context->Commands;
    Assert(Commands->CommandBufferAt <= Commands->CommandBufferSize);
    usize SizeLeft = (Commands->CommandBufferSize - Commands->CommandBufferAt);
    usize SizeRequired = sizeof(render_command);
    if (SizeLeft >= SizeRequired)
    {
        render_command *Command = (render_command *)(Commands->CommandBuffer + Commands->CommandBufferAt);
        Commands->CommandBufferAt += SizeRequired;

        Command->Type = RenderCommand_Rect;
        Command->Min = BottomLeft;
        Command->Max = BottomLeft + WidthHeight;
        Command->Color = Color;
    }
}

internal void
PushClipRect(render_context *Context, vec2 BottomLeft, vec2 WidthHeight)
{
    app_render_commands *Commands = Context->Commands;
    Assert(Commands->CommandBufferAt <= Commands->CommandBufferSize);
    usize SizeLeft = (Commands->CommandBufferSize - Commands->CommandBufferAt);
    usize SizeRequired = sizeof(render_command);
    if (SizeLeft >= SizeRequired)
    {
        render_command *Command = (render_command *)(Commands->CommandBuffer + Commands->CommandBufferAt);
        Commands->CommandBufferAt += SizeRequired;

        Command->Type = RenderCommand_ClipRect;
        Command->Min = BottomLeft;
        Command->Max = BottomLeft + WidthHeight;
    }
}
