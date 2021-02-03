#include "ray.h"

internal void RayTick(platform_api API, app_input *Input);

void
AppEntry(app_init_params *Params)
{
    Params->AppTick = RayTick;
}

internal void
RayTick(platform_api API, app_input *Input)
{
    G_Platform = API;

    local_persist ray_state *RayState = 0;
    if (!RayState)
    {
        RayState = BootstrapPushStruct(ray_state, Arena);
        RayState->Scene = PushStruct(&RayState->Arena, scene);
    }
}
