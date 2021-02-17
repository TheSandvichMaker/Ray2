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

