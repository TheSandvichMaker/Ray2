#ifndef RAY_ARENA_H
#define RAY_ARENA_H

static inline void
ZeroSize(usize SizeInit, void* DataInit)
{
    usize Size = SizeInit;
    char* Data = (char*)DataInit;
    while (Size--)
    {
        *Data++ = 0;
    }
}

#define ZeroStruct(Struct) ZeroSize(sizeof(*(Struct)), Struct)
#define ZeroArray(Count, Data) ZeroSize(sizeof(*(Data))*(Count), Data)

typedef struct arena
{
    usize Capacity;
    usize Committed;
    usize Used;
    char* Base;
    u32 TempCount;
} arena;

static inline usize
GetAlignOffset(arena* Arena, usize Align)
{
    usize Offset = (usize)Arena->Base & (Align - 1);
    if (Offset)
    {
        Offset = Align - Offset;
    }
    return Offset;
}

static inline char*
GetNextAllocationLocation(arena* Arena, usize Align)
{
    usize AlignOffset = GetAlignOffset(Arena, Align);
    char* Result = Arena->Base + Arena->Used + AlignOffset;
    return Result;
}

static inline usize
GetSizeRemaining(arena* Arena, usize Align)
{
    usize AlignOffset = GetAlignOffset(Arena, Align);
    usize Result = Arena->Capacity - (Arena->Used + AlignOffset);
    return Result;
}

static inline void
ClearArena(arena* Arena)
{
    Assert(Arena->TempCount == 0);
    Arena->Used = 0;
    Arena->TempCount = 0;
}

static inline void
DeallocateArena(arena* Arena)
{
    Assert(Arena->TempCount == 0);
    G_Platform.Deallocate(Arena->Base);
    ZeroStruct(Arena);
}

static inline void
ResetArenaTo(arena* Arena, char* Target)
{
    Assert((Target >= Arena->Base) && (Target <= (Arena->Base + Arena->Used)));
    Arena->Used = (Target - Arena->Base);
}

static inline void
InitArenaWithMemory(arena* Arena, usize MemorySize, void* Memory)
{
    ZeroStruct(Arena);
    Arena->Capacity = MemorySize;
    // NOTE: There's an assumption here that the memory passed in is valid, committed memory.
    //       If you want an arena that exploits virtual memory to progressively commit, you
    //       shouldn't init it with any existing memory.
    Arena->Committed = MemorySize;
    Arena->Base = (char*)Memory;
}

static inline void
CheckArena(arena* Arena)
{
    Assert(Arena->TempCount == 0);
}

#define PushStruct(Arena, Type) \
    (Type*)PushSize_(Arena, sizeof(Type), alignof(Type), true)
#define PushAlignedStruct(Arena, Type, Align) \
    (Type*)PushSize_(Arena, sizeof(Type), Align, true)
#define PushStructNoClear(Arena, Type) \
    (Type*)PushSize_(Arena, sizeof(Type), alignof(Type), false)
#define PushAlignedStructNoClear(Arena, Type, Align) \
    (Type*)PushSize_(Arena, sizeof(Type), Align, false)

#define PushArray(Arena, Count, Type) \
    (Type*)PushSize_(Arena, sizeof(Type)*(Count), alignof(Type), true)
#define PushAlignedArray(Arena, Count, Type, Align) \
    (Type*)PushSize_(Arena, sizeof(Type)*(Count), Align, true)
#define PushArrayNoClear(Arena, Count, Type) \
    (Type*)PushSize_(Arena, sizeof(Type)*(Count), alignof(Type), false)
#define PushAlignedArrayNoClear(Arena, Count, Type, Align) \
    (Type*)PushSize_(Arena, sizeof(Type)*(Count), Align, false)

static inline void*
PushSize_(arena* Arena, usize Size, usize Align, b32 Clear)
{
    if (!Arena->Capacity)
    {
        Assert(!Arena->Base);
        Arena->Capacity = Gigabytes(8);
    }

    if (!Arena->Base)
    {
        // NOTE: Let's align up to page size because that's the minimum allocation granularity anyway,
        //       and the code doing the commit down below assumes our capacity is page aligned.
        Arena->Capacity = AlignPow2(Arena->Capacity, G_Platform.PageSize);
        Arena->Base = (char*)G_Platform.Reserve(Arena->Capacity);
    }

    usize AlignOffset = GetAlignOffset(Arena, Align);
    usize AlignedSize = Size + AlignOffset;

    Assert((Arena->Used + AlignedSize) <= Arena->Capacity);

    char* UnalignedBase = Arena->Base + Arena->Used;

    if (Arena->Committed < (Arena->Used + AlignedSize))
    {
        usize CommitSize = AlignPow2(AlignedSize, G_Platform.PageSize);
        G_Platform.Commit(CommitSize, Arena->Base + Arena->Committed);
        Arena->Committed += CommitSize;
        Assert(Arena->Committed >= (Arena->Used + AlignedSize));
    }

    void* Result = UnalignedBase + AlignOffset;
    Arena->Used += AlignedSize;

    if (Clear) {
        ZeroSize(AlignedSize, Result);
    }

    return Result;
}

#define BootstrapPushStruct(Type, Member) \
    BootstrapPushStruct_(sizeof(Type), alignof(Type), offsetof(Type, Member))
static inline void*
BootstrapPushStruct_(usize Size, usize Align, usize ArenaOffset)
{
    arena Arena = { 0 };
    void* State = PushSize_(&Arena, Size, Align, true);
    *(arena*)((char*)State + ArenaOffset) = Arena;
    return State;
}

#endif /* RAY_ARENA_H */
