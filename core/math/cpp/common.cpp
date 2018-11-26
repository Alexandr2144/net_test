#include "core/math/common.h"

#include <stdint.h>


namespace Math {
    static const int deBruijnBitPos[] = {
        0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
        8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
    };

    int log2(int x)
    {
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        return deBruijnBitPos[(uint32_t)(x * 0x07C4ACDDU) >> 27];
    }

    // ------------------------------------------------------------------------
    int bitscount(int x)
    {
        x = (x & 0x55555555) + ((x >> 1) & 0x55555555);
        x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
        x = (x + (x >> 4)) & 0x0f0f0f0f;
        x += x >> 8;
        return (x + (x >> 16)) & 0xff;
    }

    // ------------------------------------------------------------------------
    size_t align(size_t x, size_t alignment)
    {
        size_t mask = alignment - 1;
        return size_t(x + mask) & ~mask;
    }

} // namespace Data