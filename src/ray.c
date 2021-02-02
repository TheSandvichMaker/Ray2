#include "ray.h"

static void RayTick(platform_api API, app_input* Input);

app_init_params
AppEntry(void)
{
    return (app_init_params){
        .AppTick = RayTick,
    };
}

static void
RayTick(platform_api API, app_input* Input)
{
    G_Platform = API;

    static ray_state* RayState = 0;
    if (!RayState)
    {
        RayState = BootstrapPushStruct(ray_state, Arena);
        RayState->Scene = PushStruct(&RayState->Arena, scene);
    }
}
