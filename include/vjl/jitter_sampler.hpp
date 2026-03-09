#pragma once
/// @file jitter_sampler.hpp
/// Measures scheduling/timer jitter to detect virtualization overhead.
/// Core experiment: repeatedly sleep for a target interval, measure actual
/// elapsed time, record the delta (jitter).

#include "vjl/timer.hpp"
#include <cstdint>
#include <vector>

namespace vjl {

struct JitterConfig {
    int target_us;       ///< Target sleep interval in microseconds
    std::size_t samples; ///< Number of samples to collect
    bool use_tsc;        ///< Use TSC instead of CLOCK_MONOTONIC
    int cpu_pin;         ///< CPU to pin to (-1 = no pinning)
};

struct JitterSample {
    std::int64_t target_ns; ///< What we asked for
    std::int64_t actual_ns; ///< What we got
    std::int64_t jitter_ns; ///< actual - target (positive = late)
    std::uint64_t tsc_start;
    std::uint64_t tsc_end;
};

struct JitterResult {
    std::vector<JitterSample> samples;
    std::uint64_t tsc_hz;
    bool is_virtualized;
    const char *hypervisor;
};

/// Collect jitter samples. Blocks for (target_us * samples) microseconds.
JitterResult measure_jitter(const JitterConfig &config) noexcept;

/// Run a tight-loop timing test (no sleep — measures pure scheduling preemption).
/// Each sample = time between consecutive rdtscp pairs in a busy loop.
JitterResult measure_preemption(std::size_t samples, int cpu_pin = -1) noexcept;

/// Pin current thread to a specific CPU core.
bool pin_to_cpu(int cpu) noexcept;

} // namespace vjl
