#ifndef RAY_H
#define RAY_H

#include "ray_platform.h"

global platform_api Platform;

#include "ray_arena.h"
#include "ray_handmade_math.h"

struct random_series
{
    u32 State;
};

struct accumulator
{
    u32 W, H;
    vec3 *Pixels;
};

struct plane
{
    u32 Material;
    vec3 N;
    f32 d;
};

struct sphere
{
    u32 Material;
    vec3 P;
    f32 r;
};

struct material
{
    vec3 Albedo;
};

struct camera
{
    vec3 P;
    vec3 X;
    vec3 Y;
    vec3 Z;
};

struct scene
{
    u32 MaterialCount;
    material Materials[256];

    u32 PlaneCount;
    plane Planes[256];

    u32 SphereCount;
    sphere Spheres[256];

    camera Camera;
    camera NewCamera;

    vec3 DirectionalLightD;
    vec3 DirectionalLightEmission;
};

struct common_thread_params
{
    scene *Scene;
    app_imagebuffer *Buffer;
};

struct thread_dispatch
{
    u32 ThreadCount;
    platform_semaphore_handle Semaphore;

    common_thread_params Common;

    u32 TileW, TileH;
    u32 TilesPerCol, TilesPerRow, TileCount;
    volatile u32 NextTileIndex;
    volatile u32 RetiredTileCount;
};

struct ray_state
{
    arena Arena;
    scene *Scene;

    thread_dispatch Dispatch;
};

#endif /* RAY_H */
