#ifndef RAY_H
#define RAY_H

#include "ray_platform.h"

global platform_api Platform;

#include "ray_arena.h"
#include "ray_math.h"

typedef struct plane
{
    v3 N;
    f32 d;
} plane;

typedef struct sphere
{
    v3 P;
    f32 r;
} sphere;

typedef struct scene
{
    u32 PlaneCount;
    plane Planes[256];

    u32 SphereCount;
    sphere Spheres[256];
} scene;

typedef struct thread_dispatch
{
    u32 ThreadCount;
    platform_semaphore_handle Semaphore;

    u32 TileW, TileH;
    u32 TilesPerCol, TilesPerRow, TileCount;
    volatile u32 NextTileIndex;
} thread_dispatch;

typedef struct ray_state
{
    arena Arena;
    scene *Scene;

    thread_dispatch Dispatch;
} ray_state;

#endif /* RAY_H */
