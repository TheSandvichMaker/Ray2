#include "ray.h"

#define EPSILON 0.001f

internal u32
Xorshift(random_series *Series)
{
	u32 X = Series->State;
	X ^= X << 13;
	X ^= X >> 17;
	X ^= X << 5;
	return Series->State = X;
}

internal always_inline float
UnilateralFromU32(u32 X)
{
    float Result = (float)(X >> 8)*0x1.0p-24f;
    return Result;
}

internal always_inline float
RandomUnilateral(random_series *Series)
{
    return UnilateralFromU32(Xorshift(Series));
}

internal always_inline bool
RayIntersectPlane(vec3 RayP, vec3 RayD, vec3 PlaneNormal, f32 PlaneDistance, f32 *tOut)
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
RayIntersectSphere(vec3 RayP, vec3 RayD, vec3 SphereO, f32 SphereR, f32 *tOut)
{
    bool Result = false;

    vec3 SphereRelO = RayP - SphereO;
    f32 B = Dot(RayD, SphereRelO);
    f32 C = LengthSquared(SphereRelO) - SphereR*SphereR;
    f32 Discr = (B*B - C);
    if (Discr >= 0)
    {
        f32 DiscrRoot = SquareRootF(Discr);
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

global f32 GlobalTimer;

template <bool ShadowRay>
internal always_inline bool
TraceSceneInternal(scene *Scene, vec3 RayP, vec3 RayD, f32 *tOut, u32 *OutHitMaterial, vec3 *OutHitNormal)
{
    u32 HitMaterial = 0;
    vec3 HitNormal = {};
    f32 t = *tOut;

    for (usize I = 1; I < Scene->PlaneCount; ++I)
    {
        plane *Plane = &Scene->Planes[I];
        if (RayIntersectPlane(RayP, RayD, Plane->N, Plane->d, &t))
        {
            if constexpr(ShadowRay)
            {
                return true;
            }
            HitMaterial = Plane->Material;
            HitNormal = Plane->N;
        }
    }

    for (usize I = 1; I < Scene->SphereCount; ++I)
    {
        sphere *Sphere = &Scene->Spheres[I];
        if (RayIntersectSphere(RayP, RayD, Sphere->P, Sphere->r, &t))
        {
            if constexpr(ShadowRay)
            {
                return true;
            }
            HitMaterial = Sphere->Material;
            HitNormal = Normalize(RayP + t*RayD - Sphere->P);
        }
    }

    *tOut = t;
    if (OutHitMaterial)
    {
        *OutHitMaterial = HitMaterial;
    }
    if (OutHitNormal)
    {
        *OutHitNormal = HitNormal;
    }

    return !!HitMaterial;
}

internal always_inline bool
TraceScene(scene *Scene, vec3 RayP, vec3 RayD, f32 *tOut, u32 *OutHitMaterial, vec3 *OutHitNormal)
{
    return TraceSceneInternal<false>(Scene, RayP, RayD, tOut, OutHitMaterial, OutHitNormal);
}

internal always_inline bool
Occluded(scene *Scene, vec3 RayP, vec3 RayD, f32 max_t)
{
    return TraceSceneInternal<true>(Scene, RayP, RayD, &max_t, nullptr, nullptr);
}

internal void
CastRays(scene *Scene, int MinX, int MinY, int OnePastMaxX, int OnePastMaxY, platform_backbuffer *Backbuffer)
{
    f32 RcpW = 1.0f / (f32)Backbuffer->W;
    f32 RcpH = 1.0f / (f32)Backbuffer->H;
    vec3 *Pixels = (vec3 *)Backbuffer->Pixels;

    vec3 CamP = Scene->Camera.P;
    vec3 CamX = Scene->Camera.X;
    vec3 CamY = Scene->Camera.Y;
    vec3 CamZ = Scene->Camera.Z;

    f32 FilmDistance = 1.0f;
    vec2 FilmDim = Vec2(1.0f, (f32)Backbuffer->H / (f32)Backbuffer->W);
    vec3 FilmP = CamP - CamZ*FilmDistance;

    vec3 DirectionalLightD = Scene->DirectionalLightD;
    vec3 DirectionalLightEmission = Scene->DirectionalLightEmission;

    for (ssize Y = MinY; Y < OnePastMaxY; ++Y)
    {
        f32 V = -1.0f + 2.0f*RcpH*(f32)Y;
        for (ssize X = MinX; X < OnePastMaxX; ++X)
        {
            f32 U = -1.0f + 2.0f*RcpW*(f32)X;

            vec3 RayP = CamP;
            vec3 RayD = Normalize((FilmP + FilmDim.X*CamX*U + FilmDim.Y*CamY*V) - CamP);

            f32 t = F32_MAX;
            vec3 Color = Vec3(0, 0, 1);

            u32 HitMaterial;
            vec3 HitNormal;
            if (TraceScene(Scene, RayP, RayD, &t, &HitMaterial, &HitNormal))
            {
                vec3 HitP = RayP + t*RayD;

                if (Occluded(Scene, HitP + EPSILON*DirectionalLightD, DirectionalLightD, F32_MAX))
                {
                    Color = Vec3(0, 0, 0);
                }
                else
                {
                    vec3 HitAlbedo = Scene->Materials[HitMaterial].Albedo;
                    f32 NdotL = MaxF(0.0f, Dot(DirectionalLightD, HitNormal));
                    vec3 Light = DirectionalLightEmission*NdotL;
                    vec3 ReflectedD = Reflect(RayD, HitNormal);
                    f32 Phong = MaxF(0.0f, Dot(ReflectedD, DirectionalLightD));
                    Phong *= Phong;
                    Phong *= Phong;
                    Phong *= Phong;
                    Phong *= Phong;
                    Phong *= Phong;
                    Color = HitAlbedo*Light + Phong*Light;
                }
            }

            Pixels[Y*Backbuffer->W + X] = Color;
        }
    }
}

internal void
Aim(camera *Camera, vec3 P, vec3 Z)
{
    vec3 WorldUp = Vec3(0, 1, 0);

    Camera->P = P;
    Camera->Z = Normalize(Z);
    Camera->Z.Y = Clamp(-0.95f, Camera->Z.Y, 0.95f);
    Camera->X = Normalize(Cross(WorldUp, Camera->Z));
    Camera->Y = Normalize(Cross(Camera->Z, Camera->X));
}

internal void
AimAt(camera *Camera, vec3 P, vec3 TargetP)
{
    Aim(Camera, P, P - TargetP);
}

internal void
BuildTestScene(scene *Scene)
{
    ++Scene->MaterialCount; // NOTE: NULL material
    ++Scene->PlaneCount;    // NOTE: NULL plane
    ++Scene->SphereCount;   // NOTE: NULL sphere

    AimAt(&Scene->Camera, Vec3(0, 2, -10), Vec3(0, 1, 0));

    Scene->DirectionalLightD = Normalize(Vec3(0, 1, -0.5f));
    Scene->DirectionalLightEmission = 2.0f*Vec3(1, 1, 1);

    u32 PlaneMaterialIndex = Scene->MaterialCount++;
    material *PlaneMaterial = &Scene->Materials[PlaneMaterialIndex];
    PlaneMaterial->Albedo = Vec3(0.1f, 1, 0.1f);

    u32 SphereMaterialIndex = Scene->MaterialCount++;
    material *SphereMaterial = &Scene->Materials[SphereMaterialIndex];
    SphereMaterial->Albedo = Vec3(1, 0.1f, 0.1f);

    Scene->Planes[Scene->PlaneCount++] =
    {
        .Material = PlaneMaterialIndex,
        .N = Vec3(0, 1, 0),
        .d = 0.0f,
    };

    Scene->Spheres[Scene->SphereCount++] =
    {
        .Material = SphereMaterialIndex,
        .P = Vec3(0, 2, 0),
        .r = 1.0f,
    };
}

internal void
RayThreadProc(void *UserData, platform_semaphore_handle ParentSemaphore)
{
    thread_dispatch *Dispatch = (thread_dispatch *)UserData;
    Platform.ReleaseSemaphore(ParentSemaphore, 1, nullptr);

    common_thread_params *CommonParams = &Dispatch->Common;

    for (;;)
    {
        u32 TileIndex = AtomicAddU32(&Dispatch->NextTileIndex, 1);
        if (TileIndex < Dispatch->TileCount)
        {
            scene *Scene = CommonParams->Scene;
            platform_backbuffer *Backbuffer = CommonParams->Backbuffer;

            u32 TileIndexX = TileIndex % Dispatch->TilesPerRow;
            u32 TileIndexY = TileIndex / Dispatch->TilesPerRow;
            u32 TileMinX = TileIndexX*Dispatch->TileW;
            u32 TileMinY = TileIndexY*Dispatch->TileH;
            u32 TileOnePastMaxX = MIN(TileMinX + Dispatch->TileW, Backbuffer->W);
            u32 TileOnePastMaxY = MIN(TileMinY + Dispatch->TileH, Backbuffer->H);
            CastRays(Scene, TileMinX, TileMinY, TileOnePastMaxX, TileOnePastMaxY, Backbuffer);

            AtomicAddU32(&Dispatch->RetiredTileCount, 1);
        } 
        else
        {
            Platform.WaitOnSemaphore(Dispatch->Semaphore);
        }
    }
}

internal void
InitThreadDispatcher(thread_dispatch *Dispatch)
{
    u32 ThreadCount = Platform.LogicalCoreCount;

    Dispatch->ThreadCount = ThreadCount;
    Dispatch->Semaphore = Platform.CreateSemaphore(0, ThreadCount);

    for (usize ThreadIndex = 0; ThreadIndex < ThreadCount; ++ThreadIndex)
    {
        Platform.CreateThread(RayThreadProc, Dispatch);
    }
};

internal bool
ManageDispatch(thread_dispatch *Dispatch, u32 TileW, u32 TileH, common_thread_params Common)
{
    bool Result = false;
    if (Dispatch->NextTileIndex >= Dispatch->TileCount)
    {
        Result = true;

        Dispatch->Common = Common;

        u32 W = Dispatch->Common.Backbuffer->W;
        u32 H = Dispatch->Common.Backbuffer->W;
        u32 TilesPerRow = (W + (TileW - 1)) / TileW;
        u32 TilesPerCol = (H + (TileH - 1)) / TileH;
        u32 TileCount = TilesPerRow*TilesPerCol;

        Dispatch->TileW = TileW;
        Dispatch->TileH = TileH;
        Dispatch->TilesPerRow = TilesPerRow;
        Dispatch->TilesPerCol = TilesPerCol;
        Dispatch->TileCount = TileCount;
        Dispatch->NextTileIndex = 0;
        Dispatch->RetiredTileCount = 0;

        MEMORY_BARRIER;

        int PreviousCount;
        Platform.ReleaseSemaphore(Dispatch->Semaphore, Dispatch->ThreadCount, &PreviousCount);

        Assert(PreviousCount == 0);
    }
    return Result;
}

internal void
RayInit(app_init_params *Params)
{
    Params->WindowW = 512;
    Params->WindowH = 512;
}

global ray_state *RayState = 0;

internal void
RayTick(platform_api API, app_input *Input, platform_backbuffer *Backbuffer)
{
    Platform = API;

    if (!RayState)
    {
        RayState = BootstrapPushStruct(ray_state, Arena);
        scene *Scene = RayState->Scene = PushStruct(&RayState->Arena, scene);

        InitThreadDispatcher(&RayState->Dispatch);
        BuildTestScene(Scene);
    }

    GlobalTimer += 1.0f / 240.0f;

    scene *Scene = RayState->Scene;
    AimAt(&Scene->Camera, Vec3(0, 2 + SinF(GlobalTimer), -5), Vec3(0, 1, 0));

    thread_dispatch *Dispatch = &RayState->Dispatch;
    bool Swap = ManageDispatch(Dispatch, 16, 16, common_thread_params {
        .Scene = Scene,
        .Backbuffer = Backbuffer,
    });

    Input->SwapBuffers = Swap;
}

internal void
RayExit(void)
{
    // NOTE: Wait for threads to finish before exiting the program
    thread_dispatch *Dispatch = &RayState->Dispatch;
    while (Dispatch->RetiredTileCount < Dispatch->TileCount)
    {
        _mm_pause();
    }
}

app_links
AppLinks(void)
{
    app_links Result =
    {
        .AppInit = RayInit,
        .AppTick = RayTick,
        .AppExit = RayExit,
    };
    return Result;
}
