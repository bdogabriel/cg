#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

// one contiguous mmap-reserved region; pages committed on demand
struct Arena
{
    void *base = nullptr; // start of the reserved region
    size_t used = 0;      // cursor (bytes handed out)
    size_t cap = 0;       // bytes of address space reserved (different from actual RAM committed)
};

namespace arena
{
bool init(Arena &a, size_t bytes);
void *alloc(Arena &a, size_t size, size_t align = 16);
void reset(Arena &a); // used = 0 (pages kept for reuse)
void free(Arena &a);  // munmap

// thread-local scratch arena; lazily reserved on first use
void *scratch_alloc(size_t size, size_t align = 16);
void scratch_reset();
} // namespace arena

#endif // ARENA_H
