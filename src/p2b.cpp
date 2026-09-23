#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

#include "p2a.h"
#include "timer.h"

// Dimensions of the input data
constexpr size_t DIM = 256;
// Dimensions of the kernel
constexpr size_t KDIM = 4;
// Stride of the convolution
constexpr size_t STRIDE = 4;
// Dimensions of the output data
constexpr size_t OUT_DIM = 64;

// Return the row-major index for coordinates (x, y, z)
size_t rowMajorIndex(size_t x, size_t y, size_t z) { return x * DIM * DIM + y * DIM + z; }

// Return the row-major index for coordinates (x, y, z) in the kernel
size_t rowMajorIndexK(size_t x, size_t y, size_t z) { return x * KDIM * KDIM + y * KDIM + z; }

// Return the row-major index for coordinates (x, y, z) in the output
size_t rowMajorIndexConv(size_t x, size_t y, size_t z) {
    return x * OUT_DIM * OUT_DIM + y * OUT_DIM + z;
}

int main() {
    std::mt19937_64 rng(0);

    // Phase 1: Allocate and Fill A Row Major Order
    std::vector<uint64_t> A(DIM * DIM * DIM);

    // Loop through each element in the array and fill it with a random value
    for (size_t x = 0; x < DIM; ++x) {
        for (size_t y = 0; y < DIM; ++y) {
            for (size_t z = 0; z < DIM; ++z) {
                A[rowMajorIndex(x, y, z)] = rng();
            }
        }
    }

    // Phase 2: Allocate B copy of A into Morton Order
    std::vector<uint64_t> B(DIM * DIM * DIM);
    // Loop through each element in A row major order and fill B into morton order
    for (size_t x = 0; x < DIM; ++x) {
        for (size_t y = 0; y < DIM; ++y) {
            for (size_t z = 0; z < DIM; ++z) {
                B[morton3d(x, y, z)] = A[rowMajorIndex(x, y, z)];
            }
        }
    }

    // Phase 3: Build Ka (Row Major Kernel) and Kb (Morton Order Kernel)
    std::vector<uint64_t> Ka(KDIM * KDIM * KDIM);
    std::vector<uint64_t> Kb(KDIM * KDIM * KDIM);

    // Build both kernels in row major order
    for (size_t x = 0; x < KDIM; ++x) {
        for (size_t y = 0; y < KDIM; ++y) {
            for (size_t z = 0; z < KDIM; ++z) {
                uint64_t value = x + y + z;
                Ka[rowMajorIndexK(x, y, z)] = value;
                Kb[morton3d(x, y, z)] = value;
            }
        }
    }

    // Phase 4: Convolve Ka over A row major output timed
    std::vector<uint64_t> convA(OUT_DIM * OUT_DIM * OUT_DIM);
    Timer timer;
    // Loop through each element in the output array and convolve the kernel over it
    for (size_t outX = 0; outX < OUT_DIM; ++outX) {
        for (size_t outY = 0; outY < OUT_DIM; ++outY) {
            for (size_t outZ = 0; outZ < OUT_DIM; ++outZ) {
                uint64_t sum = 0;
                // Loop through each element in the kernel and convolve it over the input array
                for (size_t kernelX = 0; kernelX < KDIM; ++kernelX) {
                    for (size_t kernelY = 0; kernelY < KDIM; ++kernelY) {
                        for (size_t kernelZ = 0; kernelZ < KDIM; ++kernelZ) {
                            // Calculate the input coordinates
                            size_t inX = outX * STRIDE + kernelX;
                            size_t inY = outY * STRIDE + kernelY;
                            size_t inZ = outZ * STRIDE + kernelZ;
                            // If the input coordinates are within bounds, add the product of the
                            // input and kernel values to the sum
                            if (inX < DIM && inY < DIM && inZ < DIM) {
                                sum += A[rowMajorIndex(inX, inY, inZ)] *
                                       Ka[rowMajorIndexK(kernelX, kernelY, kernelZ)];
                            }
                        }
                    }
                }
                // Set the output value to the sum
                convA[rowMajorIndexConv(outX, outY, outZ)] = sum;
            }
        }
    }
    uint64_t timeRowMajor = timer.click<Timer::Micros>();

    // Phase 5: Convolve Kb over B morton order output timed
    std::vector<uint64_t> convB(OUT_DIM * OUT_DIM * OUT_DIM);
    timer.restart();
    // Loop through each element in the output array and convolve the kernel over it
    for (size_t outX = 0; outX < OUT_DIM; ++outX) {
        for (size_t outY = 0; outY < OUT_DIM; ++outY) {
            for (size_t outZ = 0; outZ < OUT_DIM; ++outZ) {
                // Calculate the starting index of the block in B
                size_t blockStart = morton3d(outX * STRIDE, outY * STRIDE, outZ * STRIDE);
                uint64_t sum = 0;
                // Loop through each element in the kernel and convolve it over the input array
                for (size_t i = 0; i < 64; ++i) {
                    sum += B[blockStart + i] * Kb[i];
                }
                // Set the output value to the sum
                convB[morton3d(outX, outY, outZ)] = sum;
            }
        }
    }
    uint64_t timeMorton = timer.click<Timer::Micros>();

    // Phase 6: Verify results (runs in every build, not just debug, which also
    // forces the compiler to keep the convolution loops above from being
    // optimized away as dead code, since their outputs are genuinely read here)
    size_t mismatches = 0;
    for (size_t x = 0; x < OUT_DIM; ++x) {
        for (size_t y = 0; y < OUT_DIM; ++y) {
            for (size_t z = 0; z < OUT_DIM; ++z) {
                if (convA[rowMajorIndexConv(x, y, z)] != convB[morton3d(x, y, z)]) {
                    ++mismatches;
                }
            }
        }
    }
    assert(mismatches == 0);
    if (mismatches != 0) {
        std::cerr << "Verification failed: " << mismatches << " mismatches\n";
        std::abort();
    }

    // Phase 7: Print times
    std::cout << timeRowMajor << "\n";
    std::cout << timeMorton << "\n";

    return 0;
}