#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <cstdint>

class Timer {
public:
    // Unit type aliases
    using Nanos = std::chrono::nanoseconds;
    using Micros = std::chrono::microseconds;
    using Millis = std::chrono::milliseconds;
    using Seconds = std::chrono::seconds;
    using Minutes = std::chrono::minutes;
    using Hours = std::chrono::hours;

    // Constructor - initializes the timer by capturing the current time
    Timer() { mark_ = std::chrono::steady_clock::now(); }

    // Resets the timer to the current time
    void restart() { mark_ = std::chrono::steady_clock::now(); }

    // Measures the elapsed time since the last call to click() or restart()
    // and then resets the timer. Returns the elapsed time in the specified unit.
    template <typename T>
    uint64_t click() {
        auto temp = std::chrono::steady_clock::now();
        T elapsed = std::chrono::duration_cast<T>(temp - mark_);
        mark_ = temp;
        return elapsed.count();
    }

    // Measures the elapsed time since the last call to click() or restart() without resetting the
    // timer.
    template <typename T>
    uint64_t glance() const {
        auto temp = std::chrono::steady_clock::now();
        T elapsed = std::chrono::duration_cast<T>(temp - mark_);
        return elapsed.count();
    };

private:
    // The time point used as a reference for measuring elapsed time
    std::chrono::time_point<std::chrono::steady_clock> mark_;
};

#endif  // TIMER_H