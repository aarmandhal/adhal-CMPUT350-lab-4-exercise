#ifndef P2A_H
#define P2A_H

#include <cassert>  // assert
#include <cstddef>  // size_t
#include <cstdint>  // uint64_t, uint32_t

// Expand function from prelab
inline uint64_t expand(uint64_t input, uint32_t scale) {
    assert(scale >= 1);

    uint64_t result = 0;
    for (size_t bit = 0; bit < 64; bit++) {
        if ((input >> bit) & 1) {
            if (bit * scale < 64) {
                result |= (uint64_t)1 << (bit * scale);
            }
        }
    }
    return result;
}

inline uint64_t morton3d(uint64_t x, uint64_t y, uint64_t z) {
    // Expand each coordinate by a factor of 3 to interleave the bits
    uint64_t xx = expand(x, 3);
    // Expand y by a factor of 3
    uint64_t yy = expand(y, 3);
    // Expand z by a factor of 3
    uint64_t zz = expand(z, 3);

    // Interleave the bits of the expanded coordinates to create the morton code
    return (zz << 2) | (yy << 1) | xx;
}

#endif