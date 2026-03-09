/// @file jitter_sampler.cpp
#include "vjl/jitter_sampler.hpp"
#include "vjl/cpuid_detect.hpp"
#include <cstring>
#include <sched.h>

namespace vjl {

bool pin_to_cpu(int cpu) noexcept {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    return sched_setaffinity(0, sizeof(set), &set) == 0;
}

JitterResult measure_jitter(const JitterConfig &config) noexcept {
    JitterResult result{};
    result.tsc_hz = calibrate_tsc_hz();
    result.is_virtualized = detect_hypervisor();
    result.hypervisor = hypervisor_vendor();

    if (config.cpu_pin >= 0) {
        pin_to_cpu(config.cpu_pin);
    }

    std::int64_t target_ns = static_cast<std::int64_t>(config.target_us) * 1000;
    result.samples.reserve(config.samples);

    for (std::size_t i = 0; i < config.samples; ++i) {
        auto tsc0 = rdtsc();
        auto t0 = now_ns();

        // Sleep for target interval
        struct timespec req;
        req.tv_sec = 0;
        req.tv_nsec = target_ns;
        clock_nanosleep(CLOCK_MONOTONIC, 0, &req, nullptr);

        auto t1 = now_ns();
        auto tsc1 = rdtscp();

        JitterSample s{};
        s.target_ns = target_ns;
        s.actual_ns = t1 - t0;
        s.jitter_ns = s.actual_ns - target_ns;
        s.tsc_start = tsc0;
        s.tsc_end = tsc1;
        result.samples.push_back(s);
    }

    return result;
}

JitterResult measure_preemption(std::size_t samples, int cpu_pin) noexcept {
    JitterResult result{};
    result.tsc_hz = calibrate_tsc_hz();
    result.is_virtualized = detect_hypervisor();
    result.hypervisor = hypervisor_vendor();

    if (cpu_pin >= 0) {
        pin_to_cpu(cpu_pin);
    }

    result.samples.reserve(samples);

    // Tight loop: measure gaps between consecutive TSC reads
    auto prev_tsc = rdtscp();
    auto prev_ns = now_ns();

    for (std::size_t i = 0; i < samples; ++i) {
        auto cur_tsc = rdtscp();
        auto cur_ns = now_ns();

        JitterSample s{};
        s.target_ns = 0; // no target — measuring pure preemption
        s.actual_ns = cur_ns - prev_ns;
        s.jitter_ns = s.actual_ns; // entire elapsed is "jitter" for tight loop
        s.tsc_start = prev_tsc;
        s.tsc_end = cur_tsc;
        result.samples.push_back(s);

        prev_tsc = cur_tsc;
        prev_ns = cur_ns;
    }

    return result;
}

} // namespace vjl
