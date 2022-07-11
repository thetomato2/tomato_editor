namespace tom
{
struct Arena
{
    szt size;
    u8* base;
    szt used;
};

inline void init_arena(Arena* arena, const szt size, void* base)
{
    arena->size = size;
    arena->base = (byt*)base;
    arena->used = 0;
}

inline void* push_size(Arena* arena, szt size)
{
    TOM_ASSERT((arena->used + size) <= arena->size);
    void* result = arena->base + arena->used;
    arena->used += size;

    return result;
}

inline void zero_size(void* ptr, szt size)
{
#if USE_CRT
    memset(ptr, 0, size);
#else
    // TODO: profile this for performance
    byt* byte = (byt*)ptr;
    while (size--) {
        *byte++ = 0;
    }
#endif
}

#define PUSH_STRUCT(arena, type)       (type*)push_size(arena, sizeof(type))
#define PUSH_ARRAY(arena, count, type) (type*)push_size(arena, (count * sizeof(type)))
#define ZERO_STRUCT(inst)              zero_size(&(inst), sizeof(inst))

function void* plat_malloc(szt size)
{
    return HeapAlloc(GetProcessHeap(), 0, size);
}

function void plat_free(void* ptr)
{
    HeapFree(GetProcessHeap(), 0, ptr);
}

// NOTE: SIMPLE implmentation that will plat_free() on destruction
template<typename T>
struct ScopedPtr
{
    ScopedPtr() :
        _ptr(nullptr)
    {
    }

    ScopedPtr(T* ptr) :
        _ptr(ptr)
    {
    }

    ~ScopedPtr() { release(); }

    // no copy
    // ScopedPtr(const ScopedPtr&)            = delete;
    // ScopedPtr& operator=(const ScopedPtr&) = delete;

    // move semantics lets you return ScopedPtr from a function
    ScopedPtr(ScopedPtr&& move)
    {
        _ptr      = move._ptr;
        move._ptr = nullptr;
    }

    ScopedPtr& operator=(ScopedPtr&& move)
    {
        _ptr      = move._ptr;
        move._ptr = nullptr;
        return *this;
    }

    void release()
    {
        if (_ptr != nullptr) {
            plat_free(_ptr);
            _ptr = nullptr;
        }
    }

    T* get() { return _ptr; }
    T* get() const { return _ptr; }

    operator bool() const { return _ptr != nullptr; }

private:
    T* _ptr;
};

}  // namespace tom
