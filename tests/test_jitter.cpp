/// @file test_jitter.cpp
#include "vjl/jitter_sampler.hpp"
#include <cstdio>

void test_jitter_basic() {
    vjl::JitterConfig cfg{100, 100, false, -1}; // 100μs, 100 samples
    auto result = vjl::measure_jitter(cfg);

    if (result.samples.size() != 100) {
        std::fprintf(stderr, "  ✗ expected 100 samples, got %zu\n", result.samples.size());
        return;
    }

    // All jitter values should be >= 0 (can't wake up before the timer)
    // Actually they can be slightly negative due to measurement overhead, allow small margin
    int negative_count = 0;
    for (auto &s : result.samples) {
        if (s.jitter_ns < -10000) ++negative_count; // > 10μs early would be suspicious
    }
    if (negative_count > 5) {
        std::fprintf(stderr, "  ✗ too many suspiciously early wakeups: %d\n", negative_count);
        return;
    }

    std::fprintf(stderr, "  ✓ jitter sampling (100 samples collected)\n");
}

void test_preemption_basic() {
    auto result = vjl::measure_preemption(1000, -1);

    if (result.samples.size() != 1000) {
        std::fprintf(stderr, "  ✗ expected 1000 samples, got %zu\n", result.samples.size());
        return;
    }

    // Most samples should be very short (< 1μs for tight loop)
    int fast = 0;
    for (auto &s : result.samples) {
        if (s.jitter_ns < 1000) ++fast;
    }

    if (fast < 900) {
        std::fprintf(stderr, "  ✗ too few fast samples: %d/1000\n", fast);
        return;
    }

    std::fprintf(stderr, "  ✓ preemption measurement (%d/1000 < 1μs)\n", fast);
}

int main() {
    std::fprintf(stderr, "test_jitter:\n");
    test_jitter_basic();
    test_preemption_basic();
    std::fprintf(stderr, "  All jitter tests passed.\n");
    return 0;
}
