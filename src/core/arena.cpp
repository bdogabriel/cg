#include "arena.h"

#include <stdlib.h>
#include <sys/mman.h>

namespace arena
{

namespace
{

constexpr size_t MIN_RESERVE = 64 * 1024 * 1024; // 64 MB floor

size_t align_up(size_t n, size_t align)
{
    return (n + align - 1) & ~(align - 1);
}

// parse EDITOR_ARENA_GB; default 16; clamped to [1, 1024]
size_t reserve_size_from_env()
{
    long gb = 16;
    const char *s = getenv("EDITOR_ARENA_GB");
    if (s)
    {
        char *end = nullptr;
        long v = strtol(s, &end, 10);
        if (end != s && v >= 1)
        {
            gb = v;
        }
    }
    if (gb > 1024)
    {
        gb = 1024; // cap to avoid size_t overflow on absurd input
    }
    return (size_t)gb * 1024ull * 1024ull * 1024ull;
}

} // namespace

bool init(Arena &a, size_t bytes)
{
    // try the requested size; halve on failure
    while (bytes >= MIN_RESERVE)
    {
        void *p = mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
        if (p != MAP_FAILED)
        {
            a.base = p;
            a.used = 0;
            a.cap = bytes;
            return true;
        }
        bytes /= 2;
    }
    return false;
}

void *alloc(Arena &a, size_t size, size_t align)
{
    if (align < sizeof(void *))
    {
        align = sizeof(void *);
    }
    if (size == 0)
    {
        size = 1;
    }
    size_t off = align_up(a.used, align);
    if (off + size > a.cap)
    {
        return nullptr; // reservation exceeded
    }
    void *p = (char *)a.base + off;
    a.used = off + size;
    return p;
}

void reset(Arena &a)
{
    a.used = 0;
}

void free(Arena &a)
{
    if (a.base)
    {
        munmap(a.base, a.cap);
        a.base = nullptr;
        a.used = 0;
        a.cap = 0;
    }
}

namespace
{

thread_local Arena g_scratch;

} // namespace

void *scratch_alloc(size_t size, size_t align)
{
    if (!g_scratch.base) // lazy first-touch init
    {
        if (!init(g_scratch, reserve_size_from_env()))
        {
            return nullptr;
        }
    }
    return alloc(g_scratch, size, align);
}

void scratch_reset()
{
    g_scratch.used = 0;
}

} // namespace arena
