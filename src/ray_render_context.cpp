internal void
InitializeRenderContext(render_context *Context, app_render_commands *Commands)
{
    ZeroStruct(Context);
    Context->Commands = Commands;
}

internal void
PushRect(render_context *Context, vec2 Min, vec2 Max, vec4 Color)
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
        Command->Min = Min;
        Command->Max = Max;
        Command->Color = Color;
    }
}
