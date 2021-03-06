#include "microui.c"

#include "ray.h"
#include "ray_assets.cpp"
#include "ray_render_context.cpp"

#define EPSILON 0.001f

global bool FpsLook;
global u32 FrameIndex;

global mu_Context *Mu;
global int MuKeyMap[] =
{
    [PKC_LShift] = MU_KEY_SHIFT,
    [PKC_RShift] = MU_KEY_SHIFT,
    [PKC_LControl] = MU_KEY_CTRL,
    [PKC_RControl] = MU_KEY_CTRL,
    [PKC_LAlt] = MU_KEY_ALT,
    [PKC_RAlt] = MU_KEY_ALT,
    [PKC_Return] = MU_KEY_RETURN,
    [PKC_Backspace] = MU_KEY_BACKSPACE,
};

internal void
PipeMuInput(app_input *Input)
{
    //
    // NOTE: Mouse Input
    //

    if ((Input->MouseDeltaX != 0) ||
        (Input->MouseDeltaY != 0))
    {
        mu_input_mousemove(Mu, Input->MouseDeltaX, Input->MouseDeltaY);
    }

    if (ButtonPressed(Input->Buttons[AppButton_LeftMouse]))
    {
        mu_input_mousedown(Mu, Input->ClientMouseX, Input->ClientMouseY, MU_MOUSE_LEFT);
    }
    if (ButtonReleased(Input->Buttons[AppButton_LeftMouse]))
    {
        mu_input_mouseup(Mu, Input->ClientMouseX, Input->ClientMouseY, MU_MOUSE_LEFT);
    }
    if (ButtonPressed(Input->Buttons[AppButton_RightMouse]))
    {
        mu_input_mousedown(Mu, Input->ClientMouseX, Input->ClientMouseY, MU_MOUSE_RIGHT);
    }
    if (ButtonReleased(Input->Buttons[AppButton_RightMouse]))
    {
        mu_input_mouseup(Mu, Input->ClientMouseX, Input->ClientMouseY, MU_MOUSE_RIGHT);
    }

    //
    // NOTE: Keyboard Input
    //

    for (usize KeyEventIndex = 0; KeyEventIndex < Input->KeyEventCount; ++KeyEventIndex)
    {
        app_key_event Event = Input->KeyEvents[KeyEventIndex];

        if (Event.Pressed)
        {
            mu_input_keydown(Mu, MuKeyMap[Event.KeyCode]);
        }
        else
        {
            mu_input_keyup(Mu, MuKeyMap[Event.KeyCode]);
        }
    }
}

internal usize
CStringLength(const char *StringInit)
{
    const char *String = StringInit;
    while (*String)
    {
        ++String;
    }
    usize Result = String - StringInit;
    return Result;
}

internal int
MuTextWidth(mu_Font Font, const char *String, int Len)
{
    if (Len == -1)
    {
        Len = (int)CStringLength(String);
    }
    return 8*Len;
}

internal int
MuTextHeight(mu_Font Font)
{
    return 12;
}

internal vec4 *
GetBackbuffer(app_imagebuffer *ImageBuffer)
{
    Assert(sizeof(*ImageBuffer->Backbuffer) == sizeof(vec4));
    return (vec4 *)ImageBuffer->Backbuffer;
}

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

internal always_inline vec2
RandomUnilateralVec2(random_series *Series)
{
    return Vec2(UnilateralFromU32(Xorshift(Series)),
                UnilateralFromU32(Xorshift(Series)));
}

internal always_inline float
RandomBilateral(random_series *Series)
{
    return -1.0f + 2.0f*UnilateralFromU32(Xorshift(Series));
}

internal always_inline vec2
RandomBilateralVec2(random_series *Series)
{
    return Vec2(-1.0f + 2.0f*UnilateralFromU32(Xorshift(Series)),
                -1.0f + 2.0f*UnilateralFromU32(Xorshift(Series)));
}

internal u32
HashCoordinate(u32 X, u32 Y, u32 Z)
{
    u32 Result = ((X*73856093)^(Y*83492791)^(Z*871603259));
    return Result;
}

internal always_inline void
GetTangents(vec3 N, vec3* B1, vec3* B2)
{
    // SOURCE: https://graphics.pixar.com/library/OrthonormalB/paper.pdf
    f32 Sign = CopySignF(1.0f, N.Z);
    f32 A = -1.0f / (Sign + N.Z);
    f32 B = N.X*N.Y*A;
    *B1 = Vec3(1.0f + Sign*N.X*N.X*A, Sign*B, -Sign*N.X);
    *B2 = Vec3(B, Sign + N.Y*N.Y*A, -N.Y);
}

internal always_inline vec3
OrientedAroundNormal(vec3 V, vec3 N) {
	vec3 T, B;
	GetTangents(N, &T, &B);

    vec3 Result = (V.X*B + V.Y*N + V.Z*T);
    return Result;
}

internal always_inline vec3
MapToHemisphere(vec3 N, vec2 Sample) {
	f32 Azimuth = Tau32*Sample.X;
	f32 Y       = Sample.Y;

    vec3 Hemi;
    Hemi.X = CosF(Azimuth)*SquareRootF(1.0f - Y*Y);
    Hemi.Y = Y;
    Hemi.Z = SinF(Azimuth)*SquareRootF(1.0f - Y*Y);

    vec3 Result = OrientedAroundNormal(Hemi, N);
    return Result;
}

internal always_inline vec3
MapToCosineWeightedHemisphere(vec3 N, vec2 Sample) {
	f32 Azimuth = Tau32*Sample.X;
	f32 Y       = Sample.Y;

    vec3 Hemi;
    Hemi.X = CosF(Azimuth)*SquareRootF(1.0f - Y);
    Hemi.Y = SquareRootF(Y);
    Hemi.Z = SinF(Azimuth)*SquareRootF(1.0f - Y);

    vec3 Result = OrientedAroundNormal(Hemi, N);
    return Result;
}

internal always_inline bool
RayIntersectPlane(vec3 RayP, vec3 RayD, vec3 PlaneNormal, f32 PlaneDistance, f32 *tOut)
{
    bool Result = false;

    f32 Denom = Dot(PlaneNormal, RayD);
    if (Denom < 0.0f)
    {
        f32 t = (PlaneDistance - Dot(PlaneNormal, RayP)) / Denom;
        if ((t >= EPSILON) && (t < *tOut))
        {
            Result = true;
            *tOut = t;
        }
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

internal always_inline f32
FresnelDielectric(f32 CosThetaI, f32 EtaI, f32 EtaT, f32 EtaIOverEtaT, f32 *OutCosThetaT)
{
    // SOURCE: http://www.pbr-book.org/3ed-2018/Reflection_Models/Specular_Reflection_and_Transmission.html#fragment-ComputemonocosThetaTusingSnellslaw-0

    // NOTE: Compute CosThetaT using snell's law
    f32 SinThetaI = SquareRootF(MaxF(0.0f, 1.0f - CosThetaI*CosThetaI));
    f32 SinThetaT = EtaIOverEtaT*SinThetaI;
    f32 CosThetaT = SquareRootF(MaxF(0.0f, 1.0f - SinThetaT*SinThetaT));

    *OutCosThetaT = CosThetaT;

    // NOTE: Handle total internal reflection
    if (SinThetaT >= 1)
    {
        return 1;
    }

    f32 rParallel      = (((EtaT*CosThetaI) - (EtaI*CosThetaT)) /
                          ((EtaT*CosThetaI) + (EtaI*CosThetaT)));
    f32 rPerpendicular = (((EtaI*CosThetaI) - (EtaT*CosThetaT)) /
                          ((EtaI*CosThetaI) + (EtaT*CosThetaT)));

    f32 Result = 0.5f*(rParallel*rParallel + rPerpendicular*rPerpendicular);
    return Result;
}

internal always_inline vec3
Refract(vec3 D, vec3 N, f32 CosThetaI, f32 CosThetaT, f32 EtaIOverEtaT)
{
    vec3 Result = EtaIOverEtaT*D + N*(EtaIOverEtaT*CosThetaI - CosThetaT);
    return Result;
}

internal void
CastRays(scene *Scene, int MinX, int MinY, int OnePastMaxX, int OnePastMaxY, app_imagebuffer *ImageBuffer)
{
    u32 W = ImageBuffer->W;
    u32 H = ImageBuffer->H;
    f32 RcpW = 1.0f / (f32)W;
    f32 RcpH = 1.0f / (f32)H;
    vec4 *Pixels = GetBackbuffer(ImageBuffer);

    vec3 CamP = Scene->Camera.P;
    vec3 CamX = Scene->Camera.X;
    vec3 CamY = Scene->Camera.Y;
    vec3 CamZ = Scene->Camera.Z;

    f32 FilmDistance = 1.0f;
    vec2 FilmDim = Vec2(1.0f, (f32)H / (f32)W);
    vec3 FilmP = CamP - CamZ*FilmDistance;

    vec3 DirectionalLightD = Scene->DirectionalLightD;
    vec3 DirectionalLightEmission = Scene->DirectionalLightEmission;

    random_series Entropy = { HashCoordinate((u32)MinX, (u32)MinY, FrameIndex) };

    for (ssize Y = MinY; Y < OnePastMaxY; ++Y)
    {
        f32 V = -1.0f + 2.0f*RcpH*(f32)Y;
        for (ssize X = MinX; X < OnePastMaxX; ++X)
        {
            f32 U = -1.0f + 2.0f*RcpW*(f32)X;

            vec2 AAJitter = RandomBilateralVec2(&Entropy);
            vec2 FilmUV = Vec2(RcpW*AAJitter.X + FilmDim.X*U,
                               RcpH*AAJitter.Y + FilmDim.Y*V);

            vec3 RayP = CamP;
            vec3 RayD = Normalize(FilmP + FilmUV.X*CamX + FilmUV.Y*CamY - CamP);

            vec3 TotalColor = Vec3(0, 0, 0);
            vec3 Throughput = Vec3(1, 1, 1);

            usize MaxBounceIndex = 8;
            for (usize BounceIndex = 0; BounceIndex < MaxBounceIndex; ++BounceIndex)
            {
                f32 t = F32_MAX;
                u32 HitMaterial;
                vec3 N;
                if (TraceScene(Scene, RayP, RayD, &t, &HitMaterial, &N))
                {
                    vec3 HitP = RayP + t*RayD;
                    f32 CosThetaI = -Dot(N, RayD);

                    if (CosThetaI < 0.0f)
                    {
                        N = -N;
                        CosThetaI = -CosThetaI;
                    }

                    material *Material = &Scene->Materials[HitMaterial];

                    f32 EtaI = 1.0f;
                    f32 EtaT = Material->IOR;
                    f32 EtaIOverEtaT = EtaI / EtaT;

                    b32 IsMirror = (Material->Flags & Material_Mirror);
                    b32 ShouldReflect = IsMirror;
                    if (!ShouldReflect && (EtaI != EtaT))
                    {
                        f32 CosThetaT;
                        f32 Reflectance = FresnelDielectric(CosThetaI, EtaI, EtaT, EtaIOverEtaT, &CosThetaT);
                        f32 ReflectTest = RandomUnilateral(&Entropy);
                        ShouldReflect = (ReflectTest < Reflectance);
                    }

                    if (ShouldReflect)
                    {
                        vec3 R = Reflect(RayD, N);
                        RayP = HitP + EPSILON*R;
                        RayD = R;
                        if (IsMirror)
                        {
                            Throughput *= Material->Albedo;
                        }
                    }
                    else
                    {
                        vec3 BRDF = RcpPi32*Material->Albedo;
                        Throughput *= BRDF;

                        f32 NdotL = Dot(N, DirectionalLightD);
                        if ((NdotL > 0.0f) &&
                            !Occluded(Scene, HitP + EPSILON*DirectionalLightD, DirectionalLightD, F32_MAX))
                        {
                            TotalColor += Throughput*NdotL*DirectionalLightEmission;
                        }

                        vec3 R = MapToCosineWeightedHemisphere(N, RandomUnilateralVec2(&Entropy));

                        RayP = HitP + EPSILON*R;
                        RayD = R;

                        Throughput *= Pi32;

                        f32 RouletteTest = RandomUnilateral(&Entropy);
                        f32 RouletteChance = Clamp(0.1f, Max3(Throughput), 0.9f);
                        if (RouletteTest > RouletteChance)
                        {
                            break;
                        }
                        Throughput *= 1.0f / RouletteChance;
                    }
                }
                else
                {
                    vec3 SkyLight = Vec3(0.5f, 0.8f, 1.0f);
                    if (Scene->IBL)
                    {
                        image *IBL = Scene->IBL;

                        f32 Phi = ATan2F(RayD.Z, RayD.X);
                        f32 Theta = ASinF(RayD.Y);
                        f32 U = 0.5f + (0.5f / Pi32)*Phi;
                        f32 V = 0.5f + RcpPi32*Theta;

                        s32 SkyX = (s32)(U*(f32)IBL->W) % IBL->W;
                        s32 SkyY = (s32)(V*(f32)IBL->H) % IBL->H;

                        SkyLight = IBL->Pixels[SkyY*IBL->W + SkyX];
                    }

                    TotalColor += Throughput*SkyLight;
                    break;
                }
            }

            Pixels[Y*W + X].RGB += TotalColor;
            Pixels[Y*W + X].A   += 1;
        }
    }
}

internal void
Aim(camera *Camera, vec3 Z)
{
    vec3 WorldUp = Vec3(0, 1, 0);

    Camera->Z = Normalize(Z);
    Camera->Z.Y = Clamp(-0.9f, Camera->Z.Y, 0.9f);
    Camera->X = Normalize(Cross(WorldUp, Camera->Z));
    Camera->Y = Normalize(Cross(Camera->Z, Camera->X));
}

internal void
AimAt(camera *Camera, vec3 P, vec3 TargetP)
{
    Aim(Camera, P - TargetP);
    Camera->P = P;
}

internal u32
AddMaterial(scene *Scene, material MaterialPrototype)
{
    u32 Index = Scene->MaterialCount++;
    material *Material = &Scene->Materials[Index];
    *Material = MaterialPrototype;
    if (Max3(Material->Emissive) > 0.0f)
    {
        Material->Flags |= Material_Emissive;
    }
    return Index;
}

internal void
BuildTestScene(scene *Scene, arena *TempArena)
{
    ++Scene->MaterialCount; // NOTE: NULL material
    ++Scene->PlaneCount;    // NOTE: NULL plane
    ++Scene->SphereCount;   // NOTE: NULL sphere

    AimAt(&Scene->NewCamera, Vec3(0, 2, -5), Vec3(0, 1, 0));

    Scene->IBL = LoadHdr(&Scene->Arena, TempArena, "ballroom_4k.hdr");

    u32 PlaneMaterialIndex = AddMaterial(Scene, { .Albedo = Vec3(0.1f, 1, 0.1f) });
    u32 Plane2MaterialIndex = AddMaterial(Scene, { .Albedo = Vec3(0.8f, 0.3f, 0.5f) });
    u32 SphereMaterialIndex = AddMaterial(Scene, { .IOR = 1.5f, .Albedo = Vec3(1, 1, 1) });
    u32 Sphere2MaterialIndex = AddMaterial(Scene, { .Flags = Material_Mirror, .Albedo = Vec3(1, 0.5f, 0.2f) });

    Scene->Spheres[Scene->SphereCount++] =
    {
        .Material = SphereMaterialIndex,
        .P = Vec3(5, 5.0f, 5.5f),
        .r = 4.0f,
    };

    Scene->Spheres[Scene->SphereCount++] =
    {
        .Material = Sphere2MaterialIndex,
        .P = Vec3(5, 3.0f, 0),
        .r = 2.0f,
    };

    Scene->Spheres[Scene->SphereCount++] =
    {
        .Material = PlaneMaterialIndex,
        .P = Vec3(0, -100, 0),
        .r = 100.0f,
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
            app_imagebuffer *Buffer = CommonParams->Buffer;

            u32 TileIndexX = TileIndex % Dispatch->TilesPerRow;
            u32 TileIndexY = TileIndex / Dispatch->TilesPerRow;
            u32 TileMinX = TileIndexX*Dispatch->TileW;
            u32 TileMinY = TileIndexY*Dispatch->TileH;
            u32 TileOnePastMaxX = MIN(TileMinX + Dispatch->TileW, Buffer->W);
            u32 TileOnePastMaxY = MIN(TileMinY + Dispatch->TileH, Buffer->H);
            CastRays(Scene, TileMinX, TileMinY, TileOnePastMaxX, TileOnePastMaxY, Buffer);

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
}

internal bool
ManageDispatch(thread_dispatch *Dispatch, u32 TileW, u32 TileH, common_thread_params Common)
{
    bool Result = false;
    if (Dispatch->NextTileIndex >= Dispatch->TileCount)
    {
        Result = true;

        Dispatch->Common = Common;
        scene *Scene = Dispatch->Common.Scene;
        app_imagebuffer *Buffer = Dispatch->Common.Buffer;

        u32 W = Buffer->W;
        u32 H = Buffer->W;
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

        CopyArray(Buffer->W*Buffer->H, Buffer->Backbuffer, Buffer->Frontbuffer);
        Swap(Buffer->Backbuffer, Buffer->Frontbuffer);

        if (!StructsAreEqual(&Scene->Camera, &Scene->NewCamera))
        {
            ZeroArray(Buffer->W*Buffer->H, Buffer->Backbuffer);
            Scene->Camera = Scene->NewCamera;
        }

        FrameIndex += 1;

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
    Params->WindowW = 1280;
    Params->WindowH = 720;
}

global ray_state *RayState = 0;

internal void
RayTick(platform_api API, app_input *Input, app_imagebuffer *ImageBuffer, app_render_commands *RenderCommands)
{
    Platform = API;
    f32 dt = Input->FrameTime;

    if (!RayState)
    {
        //
        // NOTE: Initialization
        //

        RayState = BootstrapPushStruct(ray_state, Arena);
        scene *Scene = RayState->Scene = PushStruct(&RayState->Arena, scene);
        InitializeRenderContext(&RayState->RenderContext, RenderCommands);

        InitThreadDispatcher(&RayState->Dispatch);
        BuildTestScene(Scene, &RayState->Arena);

        Mu = PushStruct(&RayState->Arena, mu_Context);
        mu_init(Mu);
        Mu->text_width = MuTextWidth;
        Mu->text_height = MuTextHeight;

#if 0
        image_u32 DebugFont = LoadBitmap(&RayState->Arena, "font8x12.bmp");
        if (ValidImage(&DebugFont))
        {
            GenerateDebugFont(&DebugFont, 8, 12);
        }
#endif
    }

    //
    // NOTE: Camera Control
    //

    scene *Scene = RayState->Scene;
    camera *Camera = &Scene->NewCamera;

    if (ButtonPressed(Input->Buttons[AppButton_RightMouse]))
    {
        FpsLook = !FpsLook;
    }
    Input->CaptureCursor = FpsLook;

    if (FpsLook)
    {
        if ((Input->RawMouseDeltaX != 0) ||
            (Input->RawMouseDeltaY != 0))
        {
            float MouseScale = 1.0f;

            vec3 CameraZ = Camera->Z;
            CameraZ -= dt*MouseScale*Camera->X*(f32)Input->RawMouseDeltaX;
            CameraZ += dt*MouseScale*Camera->Y*(f32)Input->RawMouseDeltaY;

            Aim(Camera, CameraZ);
        }

        vec3 CameraP = Camera->P;

        f32 HorzMoveSpeed = 10.0f;
        f32 VertMoveSpeed = 10.0f;

        if (Input->Buttons[AppButton_Left].EndedDown)
        {
            CameraP -= dt*HorzMoveSpeed*Camera->X;
        }

        if (Input->Buttons[AppButton_Right].EndedDown)
        {
            CameraP += dt*HorzMoveSpeed*Camera->X;
        }

        if (Input->Buttons[AppButton_Forward].EndedDown)
        {
            CameraP -= dt*HorzMoveSpeed*Camera->Z;
        }

        if (Input->Buttons[AppButton_Back].EndedDown)
        {
            CameraP += dt*HorzMoveSpeed*Camera->Z;
        }

        if (Input->Buttons[AppButton_Up].EndedDown)
        {
            CameraP += dt*VertMoveSpeed*Camera->Y;
        }

        if (Input->Buttons[AppButton_Down].EndedDown)
        {
            CameraP -= dt*VertMoveSpeed*Camera->Y;
        }

        Camera->P = CameraP;
    }

    //
    // NOTE: User Interface
    //

    PipeMuInput(Input);

    mu_begin(Mu);
    if (mu_begin_window(Mu, "Hello Worldn't", mu_rect(10, 10, 320, 640)))
    {
        if (mu_button(Mu, "Hello!!!!"))
        {
            Aim(Camera, -Camera->Z);
        }
        mu_end_window(Mu);
    }
    mu_end(Mu);

    //
    // NOTE: Render User Interface
    //

    render_context *RenderContext = &RayState->RenderContext;

    mu_Command *Command = NULL;
    while (mu_next_command(Mu, &Command))
    {
        switch (Command->type)
        {
            case MU_COMMAND_CLIP:
            {
                vec2 Min = Vec2((f32)Command->clip.rect.x, (f32)ImageBuffer->H - (f32)Command->clip.rect.y - Command->clip.rect.h - 1);
                vec2 Dim = Vec2((f32)Command->clip.rect.w, (f32)Command->clip.rect.h);
                PushClipRect(RenderContext, Min, Dim);
            } break;
            case MU_COMMAND_RECT:
            {
                vec2 Min = Vec2((f32)Command->rect.rect.x, (f32)ImageBuffer->H - (f32)Command->rect.rect.y - Command->rect.rect.h - 1);
                vec2 Dim = Vec2((f32)Command->rect.rect.w, (f32)Command->rect.rect.h);
                vec4 Color = Vec4(SquareF((1.0f / 255.0f)*(f32)Command->rect.color.r),
                                  SquareF((1.0f / 255.0f)*(f32)Command->rect.color.g),
                                  SquareF((1.0f / 255.0f)*(f32)Command->rect.color.b),
                                  SquareF((1.0f / 255.0f)*(f32)Command->rect.color.a));
                PushRect(RenderContext, Min, Dim, Color);
            } break;
        }
    }

    PushRect(RenderContext,
             Vec2((f32)Input->ClientMouseX - 2, ImageBuffer->H - (f32)Input->ClientMouseY - 1 - 2),
             Vec2(5, 5),
             Vec4(1, 1, 1, 1));

    //
    // NOTE: Thread Dispatch
    //

    thread_dispatch *Dispatch = &RayState->Dispatch;
    bool FinishedPass = ManageDispatch(Dispatch, 16, 16, common_thread_params {
        .Scene = Scene,
        .Buffer = ImageBuffer,
    });
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
