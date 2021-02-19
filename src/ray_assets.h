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

struct image_u32
{
    u32 W, H;
    u32 *Pixels;
};

#pragma pack(push, 1)
struct bitmap_header {
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    s32 Width;
    s32 Height;
    u16 Planes;
    u16 BitsPerPixel;

    u32 Compression;
    u32 SizeOfBitmap;
    s32 HorzResolution;
    s32 VertResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;

    u32 RedMask;
    u32 GreenMask;
    u32 BlueMask;
    u32 AlphaMask;
};
#pragma pack(pop)

#endif /* RAY_ASSETS_H */
