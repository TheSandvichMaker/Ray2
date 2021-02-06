#include "ray.h"
#include <stdio.h>

#define EPSILON 0.001f

internal always_inline bool
RayIntersectPlane(v3 RayP, v3 RayD, v3 PlaneNormal, f32 PlaneDistance, f32 *tOut)
{
    bool Result = false;

    f32 Denom = Dot(PlaneNormal, RayD);
    f32 t = (PlaneDistance - Dot(PlaneNormal, RayP)) / Denom;
    if ((t >= EPSILON) && (t < *tOut))
    {
        Result = true;
        *tOut = t;
    }

    return Result;
}

internal always_inline bool
RayIntersectSphere(v3 RayP, v3 RayD, v3 SphereO, f32 SphereR, f32 *tOut)
{
    bool Result = false;

    v3 SphereRelO = RayP - SphereO;
    f32 B = Dot(RayD, SphereRelO);
    f32 C = LengthSq(SphereRelO) - SphereR*SphereR;
    f32 Discr = (B*B - C);
    if (Discr >= 0)
    {
        f32 DiscrRoot = SquareRoot(Discr);
        f32 tN = -B - DiscrRoot;
        f32 tF = -B + DiscrRoot;
        f32 t = (tN >= 0.0f ? tN : tF);
        if ((t >= EPSILON) && (t < *tOut))
        {
            Result = true;
            *tOut = t;
        }
    }

    return Result;
}

internal void
CastRays(scene *Scene, int MinX, int MinY, int OnePastMaxX, int OnePastMaxY, platform_backbuffer *Backbuffer)
{
    f32 RcpW = 1.0f / (f32)Backbuffer->W;
    f32 RcpH = 1.0f / (f32)Backbuffer->H;

    v3 CamP = V3(0, 2, -10);
    v3 CamX = V3(1, 0, 0);
    v3 CamY = V3(0, 1, 0);
    v3 CamZ = V3(0, 0, -1);

    f32 FilmDistance = 1.0f;
    v2 FilmDim = V2(1.0f, (f32)Backbuffer->H / (f32)Backbuffer->W);
    v3 FilmP = CamP - CamZ*FilmDistance;

    for (ssize Y = MinY; Y < OnePastMaxY; ++Y)
    {
        f32 V = -1.0f + 2.0f*RcpH*(f32)Y;
        for (ssize X = MinX; X < OnePastMaxX; ++X)
        {
            f32 U = -1.0f + 2.0f*RcpW*(f32)X;

            v3 RayP = CamP;
            v3 RayD = Normalize((FilmP + FilmDim.x*CamX*U + FilmDim.y*CamY*V) - CamP);

            f32 t = F32_MAX;

            u32 Color = 0xFF00FF00;
            for (usize I = 0; I < Scene->PlaneCount; ++I)
            {
                plane *Plane = Scene->Planes + I;
                if (RayIntersectPlane(RayP, RayD, Plane->N, Plane->d, &t))
                {
                    Color = 0xFFFF0000;
                }
            }

            for (usize I = 0; I < Scene->SphereCount; ++I)
            {
                sphere *Sphere = Scene->Spheres + I;
                if (RayIntersectSphere(RayP, RayD, Sphere->P, Sphere->r, &t))
                {
                    Color = 0xFF0000FF;
                }
            }

            Backbuffer->Pixels[Y*Backbuffer->W + X] = Color;
        }
    }
}

typedef struct ray_thread_params
{
    thread_dispatch *Dispatch;
    scene *Scene;
    platform_backbuffer *Backbuffer;
} ray_thread_params;

internal void
RayThreadProc(void *UserData, platform_semaphore_handle ParentSemaphore)
{
    ray_thread_params *Params = (ray_thread_params *)UserData;

    thread_dispatch *Dispatch = Params->Dispatch;
    scene *Scene = Params->Scene;
    platform_backbuffer *Backbuffer = Params->Backbuffer;

    Platform.ReleaseSemaphore(ParentSemaphore, 1, NULL);

    for (;;)
    {
        u32 TileIndex = AtomicAddU32(&Dispatch->NextTileIndex, 1);
        if (TileIndex < Dispatch->TileCount)
        {
            u32 TileIndexX = TileIndex % Dispatch->TilesPerRow;
            u32 TileIndexY = TileIndex / Dispatch->TilesPerRow;
            u32 TileMinX = TileIndexX*Dispatch->TileW;
            u32 TileMinY = TileIndexY*Dispatch->TileH;
            u32 TileOnePastMaxX = MIN(TileMinX + Dispatch->TileW, Backbuffer->W);
            u32 TileOnePastMaxY = MIN(TileMinY + Dispatch->TileH, Backbuffer->H);
            CastRays(Scene, TileMinX, TileMinY, TileOnePastMaxX, TileOnePastMaxY, Backbuffer);
        } 
        else
        {
            Platform.WaitOnSemaphore(Dispatch->Semaphore);
        }
    }
}

internal void
InitThreadDispatcher(thread_dispatch *Dispatch,
                     scene *Scene,
                     platform_backbuffer *Backbuffer,
                     u32 TileW,
                     u32 TileH)
{
    u32 W = Backbuffer->W;
    u32 H = Backbuffer->W;
    u32 TilesPerRow = (W + (TileW - 1)) / TileW;
    u32 TilesPerCol = (H + (TileH - 1)) / TileH;
    u32 TileCount = TilesPerRow*TilesPerCol;

    Dispatch->TileW = TileW;
    Dispatch->TileH = TileH;
    Dispatch->TilesPerRow = TilesPerRow;
    Dispatch->TilesPerCol = TilesPerCol;
    Dispatch->TileCount = TileCount;
    Dispatch->NextTileIndex = TileCount; // NOTE: Start in an idle state

    u32 ThreadCount = Platform.LogicalCoreCount;

    Dispatch->ThreadCount = ThreadCount;
    Dispatch->Semaphore = Platform.CreateSemaphore(0, ThreadCount);

    for (usize ThreadIndex = 0; ThreadIndex < ThreadCount; ++ThreadIndex)
    {
        ray_thread_params Params =
        {
            .Dispatch = Dispatch,
            .Scene = Scene,
            .Backbuffer = Backbuffer,
        };
        Platform.CreateThread(RayThreadProc, &Params);
    }
};

internal void
ManageDispatch(thread_dispatch *Dispatch)
{
    if (Dispatch->NextTileIndex >= Dispatch->TileCount)
    {
        Dispatch->NextTileIndex = 0;

        MEMORY_BARRIER;

        int PreviousCount;
        Platform.ReleaseSemaphore(Dispatch->Semaphore, Dispatch->ThreadCount, &PreviousCount);

        Assert(PreviousCount == 0);
    }
}

internal void
RayInit(app_init_params *Params)
{
    Params->WindowTitle = "Hello Ray2";
    Params->WindowW = 512;
    Params->WindowH = 512;
}

internal void
RayTick(platform_api API, app_input *Input, platform_backbuffer *Backbuffer)
{
    Platform = API;

    local_persist ray_state *RayState = 0;
    if (!RayState)
    {
        RayState = BootstrapPushStruct(ray_state, Arena);
        RayState->Scene = PushStruct(&RayState->Arena, scene);

        InitThreadDispatcher(&RayState->Dispatch, RayState->Scene, Backbuffer, 16, 16);

        scene *Scene = RayState->Scene;
        Scene->Planes[Scene->PlaneCount++] =
        (plane) {
            .N = V3(0, 1, 0),
            .d = 0.0f,
        };

        Scene->Spheres[Scene->SphereCount++] =
        (sphere) {
            .P = V3(0, 2, 0),
            .r = 1.0f,
        };
    }

    thread_dispatch *Dispatch = &RayState->Dispatch;
    ManageDispatch(Dispatch);
}

app_links
AppLinks(void)
{
    app_links Result =
    {
        .AppInit = RayInit,
        .AppTick = RayTick,
    };
    return Result;
}
