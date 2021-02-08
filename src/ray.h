#ifndef RAY_H
#define RAY_H

#include "ray_platform.h"

global platform_api Platform;

#include "ray_arena.h"
// #include "ray_math.h"
#include "ray_handmade_math.h"

struct plane
{
    vec3 N;
    f32 d;
};

struct sphere
{
    vec3 P;
    f32 r;
};

struct scene
{
    u32 PlaneCount;
    plane Planes[256];

    u32 SphereCount;
    sphere Spheres[256];
};

struct thread_dispatch
{
    u32 ThreadCount;
    platform_semaphore_handle Semaphore;

    u32 TileW, TileH;
    u32 TilesPerCol, TilesPerRow, TileCount;
    volatile u32 NextTileIndex;
};

struct ray_state
{
    arena Arena;
    scene *Scene;

    thread_dispatch Dispatch;
};

#endif /* RAY_H */
