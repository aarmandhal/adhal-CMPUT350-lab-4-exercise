#include <chrono>
#include <cstddef>  // size_t
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

#include "timer.h"

constexpr size_t N = 4000;
constexpr size_t TOTAL = N * N;

size_t rowMajorIndex(size_t row, size_t col) { return row * N + col; }

int main() {
    std::mt19937_64 rng(0);

    // Two contiguous arrays
    std::vector<uint64_t> arr1(TOTAL);
    std::vector<uint64_t> arr2(TOTAL);

    // Fill array 1 in row major order
    for (size_t row = 0; row < N; ++row) {
        for (size_t col = 0; col < N; ++col) {
            arr1[rowMajorIndex(row, col)] = rng();
        }
    }
    rng.seed(0);

    // Fill array 2 in column major order
    for (size_t col = 0; col < N; ++col) {
        for (size_t row = 0; row < N; ++row) {
            arr2[rowMajorIndex(row, col)] = rng();
        }
    }

    // Sum array 1 in row major order and time it
    Timer timer1;
    uint64_t sum1 = 0;
    for (size_t row = 0; row < N; ++row) {
        for (size_t col = 0; col < N; ++col) {
            sum1 += arr1[rowMajorIndex(row, col)];
        }
    }
    uint64_t time1 = timer1.click<Timer::Micros>();
    std::cout << time1 << " " << sum1 << "\n";

    // Sum array 2 in column major order and time it
    Timer timer2;
    uint64_t sum2 = 0;
    for (size_t col = 0; col < N; ++col) {
        for (size_t row = 0; row < N; ++row) {
            sum2 += arr2[rowMajorIndex(row, col)];
        }
    }
    uint64_t time2 = timer2.click<Timer::Micros>();
    std::cout << time2 << " " << sum2 << "\n";

    return 0;
}
