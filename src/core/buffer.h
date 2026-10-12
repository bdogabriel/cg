#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdlib.h>

// contiguous growable array
// holds a raw allocation and its capacity in elements
// count is tracked by the owning struct
struct Buffer
{
    void *data = nullptr;
    size_t elemSize = 0;
    int cap = 0; // capacity, in elements
};

// typed access
#define BUF(T, b) ((T *)((b).data))

namespace buffer
{

inline void ensure(Buffer &b, size_t elemSize, int need)
{
    if (need <= b.cap)
    {
        return;
    }
    int newCap = b.cap > 0 ? b.cap * 2 : 16;
    if (newCap < need)
    {
        newCap = need;
    }
    void *nd = realloc(b.data, (size_t)newCap * elemSize);
    if (!nd)
    {
        return;
    }
    b.data = nd;
    b.elemSize = elemSize;
    b.cap = newCap;
}

inline void free(Buffer &b)
{
    ::free(b.data);
    b.data = nullptr;
    b.elemSize = 0;
    b.cap = 0;
}

inline void swap(Buffer &a, Buffer &b)
{
    Buffer t = a;
    a = b;
    b = t;
}

} // namespace buffer

#endif // BUFFER_H
