#ifndef RAY_ASSETS_H
#define RAY_ASSETS_H

enum radiance_format
{
    Radiance_RGB,
    Radiance_XYZ,
};

union radiance_color
{
    struct
    {
        u8 R, G, B, Exp;
    };
    u8 E[4];
};

struct image
{
    u32 W, H;
    vec3 *Pixels;
};

#endif /* RAY_ASSETS_H */
