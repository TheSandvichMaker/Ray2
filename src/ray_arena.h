#ifndef RAY_ARENA_H
#define RAY_ARENA_H

internal inline void
ZeroSize(usize SizeInit, void *DataInit)
{
    usize Size = SizeInit;
    char *Data = (char *)DataInit;
    while (Size--)
    {
        *Data++ = 0;
    }
}

#define ZeroStruct(Struct) ZeroSize(sizeof(*(Struct)), Struct)
#define ZeroArray(Count, Data) ZeroSize(sizeof(*(Data))*(Count), Data)

internal bool
MemoryIsEqual(usize Count, void *AInit, void *BInit)
{
    char *A = (char *)AInit;
    char *B = (char *)BInit;

    bool Result = true;
    while (Count--)
    {
        if (*A++ != *B++)
        {
            Result = false;
            break;
        }
    }
    return Result;
}

#define StructsAreEqual(A, B) (Assert(sizeof(*(A)) == sizeof(*(B))), MemoryIsEqual(sizeof(*(A)), A, B))

internal void
CopySize(usize Size, void *SourceInit, void *DestInit)
{
    char *Source = (char *)SourceInit;
    char *Dest = (char *)DestInit;
    while (--Size)
    {
        *Dest++ = *Source++;
    }
}

#define CopyArray(Count, Source, Dest) CopySize(sizeof(*(Source))*Count, Source, Dest)

#define DEFAULT_ARENA_CAPACITY Gigabytes(8)

typedef struct arena
{
    usize Capacity;
    usize Committed;
    usize Used;
    char *Base;
    u32 TempCount;
} arena;

internal inline usize
GetAlignOffset(arena *Arena, usize Align)
{
    usize Offset = (usize)(Arena->Base + Arena->Used) & (Align - 1);
    if (Offset)
    {
        Offset = Align - Offset;
    }
    return Offset;
}

internal inline char *
GetNextAllocationLocation(arena *Arena, usize Align)
{
    usize AlignOffset = GetAlignOffset(Arena, Align);
    char* Result = Arena->Base + Arena->Used + AlignOffset;
    return Result;
}

internal inline usize
GetSizeRemaining(arena *Arena, usize Align)
{
    usize AlignOffset = GetAlignOffset(Arena, Align);
    usize Result = Arena->Capacity - (Arena->Used + AlignOffset);
    return Result;
}

internal inline void
ClearArena(arena *Arena)
{
    Assert(Arena->TempCount == 0);
    Arena->Used = 0;
    Arena->TempCount = 0;
}

internal inline void
DeallocateArena(arena *Arena)
{
    Assert(Arena->TempCount == 0);
    Platform.Deallocate(Arena->Base);
    ZeroStruct(Arena);
}

internal inline void
ResetArenaTo(arena *Arena, char *Target)
{
    Assert((Target >= Arena->Base) && (Target <= (Arena->Base + Arena->Used)));
    Arena->Used = (Target - Arena->Base);
}

internal inline void
InitArenaWithMemory(arena *Arena, usize MemorySize, void *Memory)
{
    ZeroStruct(Arena);
    Arena->Capacity = MemorySize;
    // NOTE: There's an assumption here that the memory passed in is valid, committed memory.
    //       If you want an arena that exploits virtual memory to progressively commit, you
    //       shouldn't init it with any existing memory.
    Arena->Committed = MemorySize;
    Arena->Base = (char *)Memory;
}

internal inline void
CheckArena(arena* Arena)
{
    Assert(Arena->TempCount == 0);
}

#define PushStruct(Arena, Type) \
    (Type *)PushSize_(Arena, sizeof(Type), alignof(Type), true, LOCATION_STRING(#Arena))
#define PushAlignedStruct(Arena, Type, Align) \
    (Type *)PushSize_(Arena, sizeof(Type), Align, true, LOCATION_STRING(#Arena))
#define PushStructNoClear(Arena, Type) \
    (Type *)PushSize_(Arena, sizeof(Type), alignof(Type), false, LOCATION_STRING(#Arena))
#define PushAlignedStructNoClear(Arena, Type, Align) \
    (Type *)PushSize_(Arena, sizeof(Type), Align, false, LOCATION_STRING(#Arena))

#define PushArray(Arena, Count, Type) \
    (Type *)PushSize_(Arena, sizeof(Type)*(Count), alignof(Type), true, LOCATION_STRING(#Arena))
#define PushAlignedArray(Arena, Count, Type, Align) \
    (Type *)PushSize_(Arena, sizeof(Type)*(Count), Align, true, LOCATION_STRING(#Arena))
#define PushArrayNoClear(Arena, Count, Type) \
    (Type *)PushSize_(Arena, sizeof(Type)*(Count), alignof(Type), false, LOCATION_STRING(#Arena))
#define PushAlignedArrayNoClear(Arena, Count, Type, Align) \
    (Type *)PushSize_(Arena, sizeof(Type)*(Count), Align, false, LOCATION_STRING(#Arena))

internal inline void *
PushSize_(arena *Arena, usize Size, usize Align, b32 Clear, const char *Tag)
{
    if (!Arena->Capacity)
    {
        Assert(!Arena->Base);
        Arena->Capacity = DEFAULT_ARENA_CAPACITY;
    }

    if (!Arena->Base)
    {
        // NOTE: Let's align up to page size because that's the minimum allocation granularity anyway,
        //       and the code doing the commit down below assumes our capacity is page aligned.
        Arena->Capacity = AlignPow2(Arena->Capacity, Platform.PageSize);
        Arena->Base = (char *)Platform.Reserve(Arena->Capacity, MemFlag_NoLeakCheck, Tag);
    }

    usize AlignOffset = GetAlignOffset(Arena, Align);
    usize AlignedSize = Size + AlignOffset;

    Assert((Arena->Used + AlignedSize) <= Arena->Capacity);

    char *UnalignedBase = Arena->Base + Arena->Used;

    if (Arena->Committed < (Arena->Used + AlignedSize))
    {
        usize CommitSize = AlignPow2(AlignedSize, Platform.PageSize);
        Platform.Commit(CommitSize, Arena->Base + Arena->Committed);
        Arena->Committed += CommitSize;
        Assert(Arena->Committed >= (Arena->Used + AlignedSize));
    }

    void *Result = UnalignedBase + AlignOffset;
    Arena->Used += AlignedSize;

    if (Clear) {
        ZeroSize(AlignedSize, Result);
    }

    return Result;
}

#define BootstrapPushStruct(Type, Member)                                             \
    (Type *)BootstrapPushStruct_(sizeof(Type), alignof(Type), offsetof(Type, Member), \
                                 LOCATION_STRING("Bootstrap " #Type "::" #Member))
internal inline void *
BootstrapPushStruct_(usize Size, usize Align, usize ArenaOffset, const char *Tag)
{
    arena Arena = {};
    void *State = PushSize_(&Arena, Size, Align, true, Tag);
    *(arena *)((char *)State + ArenaOffset) = Arena;
    return State;
}

typedef struct temporary_memory
{
    arena *Arena;
    usize Used;
} temporary_memory;

internal inline temporary_memory
BeginTemporaryMemory(arena *Arena)
{
    temporary_memory Result =
    {
        .Arena = Arena,
        .Used  = Arena->Used,
    };
    ++Arena->TempCount;
    return Result;
}

internal inline void
EndTemporaryMemory(temporary_memory Temp)
{
    if (Temp.Arena)
    {
        Temp.Arena->Used = Temp.Used;
        --Temp.Arena->TempCount;
    }
}

#define ScopedMemory(TempArena)                                       \
    for (temporary_memory TempMem_ = BeginTemporaryMemory(TempArena); \
         TempMem_.Arena;                                              \
         EndTemporaryMemory(TempMem_), TempMem_.Arena = 0)

#endif /* RAY_ARENA_H */
