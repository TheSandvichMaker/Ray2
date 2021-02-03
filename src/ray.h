#ifndef RAY_H
#define RAY_H

#include "ray_platform.h"

global platform_api G_Platform;

#include "ray_arena.h"
#include "ray_math.h"

typedef struct plane {
    v3 N;
    f32 d;
} plane;

typedef struct sphere {
    v3 P;
    f32 R;
} sphere;

typedef struct scene {
    u32 PlaneCount;
    plane Planes[256];

    u32 SphereCount;
    sphere Spheres[256];
} scene;

typedef struct ray_state
{
    arena Arena;
    scene *Scene;
} ray_state;

#endif /* RAY_H */
