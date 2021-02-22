#define MatchLiteral(Stream, Literal) MatchWord(Stream, sizeof(Literal) - 1, Literal)
internal bool
MatchWord(char **Stream, usize Len, char *Word)
{
    bool Result = true;
    char *At = *Stream;

    while (*At && *At == ' ')
    {
        ++At;
    }

    for (usize CharIndex = 0; CharIndex < Len; ++CharIndex)
    {
        if (!*At || *At != *Word)
        {
            Result = false;
            break;
        }
        ++At;
        ++Word;
    }

    if (Result)
    {
        *Stream = At;
    }

    return Result;
}

internal bool
ParseF32(char **Stream, f32 *OutValue)
{
    bool Result = false;

    char *End;
    f32 Value = strtof(*Stream, &End);

    if (End != *Stream)
    {
        Result = true;
        *Stream = End;
        *OutValue = Value;
    }

    return Result;
}

internal bool
ParseU32(char **Stream, u32 *OutValue)
{
    bool Result = false;

    char *End;
    u32 Value = (u32)strtoul(*Stream, &End, 0);

    if (End != *Stream)
    {
        Result = true;
        *Stream = End;
        *OutValue = Value;
    }

    return Result;
}

internal f32
FloatFromBits(u32 Bits)
{
    return (f32&)Bits;
}

internal vec3
DecodeRadianceColor(radiance_color Col)
{
    vec3 Result = {};
    if (Col.Exp > 9)
    {
        f32 Mul = FloatFromBits((Col.Exp - 9) << 23);
        Result = Vec3(Mul*((f32)Col.R + 0.5f),
                      Mul*((f32)Col.G + 0.5f),
                      Mul*((f32)Col.B + 0.5f));
    }
    return Result;
}

internal image *
ParseHdr(arena *Arena, arena *TempArena, char *Input)
{
    temporary_memory ResultTemp = BeginTemporaryMemory(Arena);
    image *Result = PushStruct(Arena, image);

    s32 AdvanceX =  1;
    s32 AdvanceY = -1;

    Result->W = 0;
    Result->H = 0;

    radiance_format Format = Radiance_RGB;
    vec2 PrimaryR = {};
    vec2 PrimaryG = {};
    vec2 PrimaryB = {};
    vec2 PrimaryW = {};

    char *At = Input;
    // NOTE: Parse header
    while (*At)
    {
        if (*At == '\n')
        {
            // NOTE: End of header
            ++At;
            break;
        }
        else
        {
            if (MatchLiteral(&At, "FORMAT"))
            {
                if (!MatchLiteral(&At, "="))
                {
                    fprintf(stderr, "HDR PARSE ERROR: Malformed header.\n");
                    goto Bail;
                }
                if (MatchLiteral(&At, "32-bit_rle_rgbe"))
                {
                    Format = Radiance_RGB;
                }
                else if (MatchLiteral(&At, "32-bit_rle_xyz"))
                {
                    Format = Radiance_XYZ;
                }
                else
                {
                    fprintf(stderr, "HDR PARSE WARNING: Unknown FORMAT, defaulting to RGB\n");
                }
            }
            else if (MatchLiteral(&At, "PRIMARIES"))
            {
                if (!MatchLiteral(&At, "="))
                {
                    fprintf(stderr, "HDR PARSE ERROR: Malformed header.\n");
                    goto Bail;
                }
                if (!ParseF32(&At, &PrimaryR.X) ||
                    !ParseF32(&At, &PrimaryR.Y) ||
                    !ParseF32(&At, &PrimaryG.X) ||
                    !ParseF32(&At, &PrimaryG.Y) ||
                    !ParseF32(&At, &PrimaryB.X) ||
                    !ParseF32(&At, &PrimaryB.Y) ||
                    !ParseF32(&At, &PrimaryW.X) ||
                    !ParseF32(&At, &PrimaryW.Y))
                {
                    fprintf(stderr, "HDR PARSE WARNING: Failed to correctly parse PRIMARIES. Using defaults.\n");
                    PrimaryR = Vec2(0.640f, 0.330f);
                    PrimaryG = Vec2(0.290f, 0.600f);
                    PrimaryB = Vec2(0.150f, 0.060f);
                    PrimaryW = Vec2(0.333f, 0.333f);
                }
            }
        }

        // NOTE: Skip unhandled lines
        while (*At && *At != '\n')
        {
            ++At;
        }
        if (*At == '\n')
        {
            ++At;
        }
    }

    if (Format == Radiance_XYZ) {
        fprintf(stderr, "HDR PARSE WARNING: XYZ encoding is not handled. Who knows what your image will look like.\n");
    }

    if (!*At) {
        fprintf(stderr, "HDR PARSE ERROR: Unexpected end of file while parsing header.\n");
        goto Bail;
    }

    // NOTE: Parse resolution string
    if (MatchLiteral(&At, "+Y"))
    {
        AdvanceY =  1;
    }
    else if (MatchLiteral(&At, "-Y"))
    {
        AdvanceY = -1;
    }
    else
    {
        fprintf(stderr, "HDR PARSE ERROR: Failed to parse resolution string (+/-Y).\n");
        goto Bail;
    }

    if (!ParseU32(&At, &Result->H))
    {
        fprintf(stderr, "HDR PARSE ERROR: Failed to parse vertical resolution.\n");
        goto Bail;
    }

    if (MatchLiteral(&At, "+X"))
    {
        AdvanceX =  1;
    }
    else if (MatchLiteral(&At, "-X"))
    {
        AdvanceX = -1;
    }
    else
    {
        fprintf(stderr, "HDR PARSE ERROR: Failed to parse resolution string (+/-X).\n");
        goto Bail;
    }

    if (!ParseU32(&At, &Result->W))
    {
        fprintf(stderr, "HDR PARSE ERROR: Failed to parse horizontal resolution.\n");
        goto Bail;
    }

    if (*At++ != '\n')
    {
        fprintf(stderr, "HDR PARSE ERROR: Expected newline after resolution string.\n");
        goto Bail;
    }

    if (Result->W && Result->H)
    {
        temporary_memory Temp = BeginTemporaryMemory(TempArena);

        radiance_color *RadiancePixels = PushArray(TempArena, (usize)Result->W*Result->H, radiance_color);
        radiance_color *DstRow = RadiancePixels;

        if (AdvanceX < 0)
        {
            DstRow += (usize)Result->W - 1;
        }
        if (AdvanceY < 0)
        {
            DstRow += (usize)Result->W*(Result->H - 1);
        }
        for (usize Y = 0; Y < Result->H; ++Y)
        {
            u16 Signature = (At[0] << 8)|At[1];
            At += 2;

            if (Signature != 0x0202)
            {
                fprintf(stderr, "HDR PARSE ERROR: .hdr format unsupported.\n");
                goto Bail;
            }

            u16 ScanlineLength = (At[0] << 8)|At[1];
            At += 2;

            if (ScanlineLength != Result->W)
            {
                fprintf(stderr, "HDR PARSE ERROR: Scanline length did not match image width.\n");
                goto Bail;
            }

            // NOTE: Yes, that work up above is completely useless, this format is a bit shit.

            for (usize ChannelIndex = 0; ChannelIndex < 4; ++ChannelIndex)
            {
                radiance_color *Dst = DstRow;
                for (usize X = 0; X < Result->W;)
                {
                    u8 Code = *At++;
                    if (Code > 128)
                    {
                        // NOTE: Run
                        u8 RunCount = Code & 127;
                        u8 RunValue = *At++;
                        while (RunCount--)
                        {
                            Assert(X < Result->W);
                            Dst->E[ChannelIndex] = RunValue;
                            Dst += AdvanceX;
                            ++X;
                        }
                    }
                    else
                    {
                        // NOTE: Literals
                        u8 LiteralCount = Code;
                        while (LiteralCount--)
                        {
                            Assert(X < Result->W);
                            Dst->E[ChannelIndex] = *At++;
                            Dst += AdvanceX;
                            ++X;
                        }
                    }
                }
            }

            DstRow += AdvanceY*(ssize)Result->W;
        }

        Result->Pixels = PushArrayNoClear(Arena, Result->W*Result->H, vec3);

        radiance_color *Src = RadiancePixels;
        vec3 *Dst = Result->Pixels;
        for (usize I = 0; I < (usize)Result->W*Result->H; ++I)
        {
            *Dst++ = DecodeRadianceColor(*Src++);
        }

        EndTemporaryMemory(Temp);
    }
    else
    {
        fprintf(stderr, "HDR PARSE ERROR: Malformed resolution.\n");
        goto Bail;
    }

    if (*At != '\0')
    {
        fprintf(stderr, "HDR PARSE WARNING: Expected end of file, but there's more!\n");
    }

    CommitTemporaryMemory(&ResultTemp);

Bail:
    if (!Result)
    {
        Result = NULL;
    }

    return Result;
}

internal image *
LoadHdr(arena *Arena, arena *TempArena, const char *FileName)
{
    image *Result = NULL;
    ScopedMemory(TempArena)
    {
        string_u8 File = Platform.ReadEntireFile(TempArena, FileName);
        if (File.Count)
        {
            Result = ParseHdr(Arena, TempArena, (char *)File.Data);
        }
        else
        {
            fprintf(stderr, "Could not open %s\n", FileName);
        }
    }
    return Result;
}

typedef struct bit_scan_result {
    u32 Found;
    u32 Index;
} bit_scan_result;

internal bit_scan_result
FindLeastSignificantSetBit(u32 Value)
{
    bit_scan_result Result = {};
    
    for (u32 Test = 0; Test < 32; ++Test)
    {
        if (Value & (1 << Test))
        {
            Result.Index = Test;
            Result.Found = true;
            break;
        }
    }

    return Result;
}

internal bool
ParseBitmap(size_t SourceSize, void *SourceData, image_u32 *Image)
{
    bool Result = false;
    ZeroStruct(Image);

    if ((SourceSize > 0) && SourceData)
    {
        bitmap_header *Header = (bitmap_header *)SourceData;

        Assert(Header->Size >= sizeof(Header));
        Assert(Header->Height >= 0);
        Assert(Header->Compression == 3);

        u32 *Pixels = (u32 *)((u8 *)Header + Header->BitmapOffset);
        Image->W = Header->Width;
        Image->H = Header->Height;

        u32 AlphaMask = Header->AlphaMask;
        u32 RedMask = Header->RedMask;
        u32 GreenMask = Header->GreenMask;
        u32 BlueMask = Header->BlueMask;

        if (!AlphaMask)
        {
            AlphaMask = ~(RedMask|GreenMask|BlueMask);
        }

        bit_scan_result AlphaScan = FindLeastSignificantSetBit(AlphaMask);
        bit_scan_result RedScan = FindLeastSignificantSetBit(RedMask);
        bit_scan_result GreenScan = FindLeastSignificantSetBit(GreenMask);
        bit_scan_result BlueScan = FindLeastSignificantSetBit(BlueMask);

        Assert(AlphaScan.Found &&
               RedScan.Found &&
               GreenScan.Found &&
               BlueScan.Found);

        s32 AlphaShiftDown = (s32)AlphaScan.Index;
        s32 RedShiftDown = (s32)RedScan.Index;
        s32 GreenShiftDown = (s32)GreenScan.Index;
        s32 BlueShiftDown = (s32)BlueScan.Index;

        s32 AlphaShiftUp = 24;
        s32 RedShiftUp = 16;
        s32 GreenShiftUp = 8;
        s32 BlueShiftUp = 0;

        u32 *SourceDest = Pixels;
        for (s32 I = 0; I < Header->Width*Header->Height; ++I)
        {
            u32 C = *SourceDest;
            f32 TexelR = (f32)((C & RedMask) >> RedShiftDown);
            f32 TexelG = (f32)((C & GreenMask) >> GreenShiftDown);
            f32 TexelB = (f32)((C & BlueMask) >> BlueShiftDown);
            f32 TexelA = (f32)((C & AlphaMask) >> AlphaShiftDown);

            f32 Rcp255 = 1.0f/255.0f;
            TexelA = Rcp255*TexelA;
            TexelR = 255.0f*SquareRootF(SquareF(Rcp255*TexelR)*TexelA);
            TexelG = 255.0f*SquareRootF(SquareF(Rcp255*TexelG)*TexelA);
            TexelB = 255.0f*SquareRootF(SquareF(Rcp255*TexelB)*TexelA);
            TexelA *= 255.0f;

            *SourceDest++ = (((u32)(TexelA + 0.5f) << AlphaShiftUp)|
                             ((u32)(TexelR + 0.5f) << RedShiftUp)|
                             ((u32)(TexelG + 0.5f) << GreenShiftUp)|
                             ((u32)(TexelB + 0.5f) << BlueShiftUp));
        }

        Image->Pixels = Pixels;
        Result = true;
    }

    return Result;
}

internal image_u32
LoadBitmap(arena *Arena, const char *FileName)
{
    image_u32 Result = {};

    ScopedMemory(Arena)
    {
        string_u8 File = Platform.ReadEntireFile(Arena, FileName);
        if (File.Count)
        {
            if (ParseBitmap(File.Count, File.Data, &Result))
            {
                CommitTemporaryMemory(&ScopeMemory);
            }
        }
    }

    return Result;
}

internal void
GenerateDebugFont(image_u32 *Image, int GlyphW, int GlyphH)
{
    // TODO: fix. i am dead
    int GlyphsPerRow = Image->W % GlyphW;
    int GlyphsPerCol = Image->W / GlyphH;
    u32 *At = Image->Pixels + Image->W*(Image->H - GlyphH - 1);
    printf("global u32 DebugFontData[][] =\n");
    printf("{\n");
    while (At > Image->Pixels)
    {
        printf("    {\n");
        for (int Y = 0; Y < GlyphH; ++Y)
        {
            printf("        ");
            for (int X = 0; X < GlyphW; ++X)
            {
                u32 Color = At[Y*Image->W + X];
                // NOTE: magenta = transparency
                if (Color == 0xFFFF00FF)
                {
                    Color = 0;
                }
                printf("%X, ", Color);
            }
            printf("\n");
        }
        printf("    },\n");

        At += GlyphW;
    }
    printf("};\n");

    printf(R"(
global image DebugFont =
{
    %u, %u,
    (u32 *)DebugFontData,
};)", Image->W, Image->H);
}
