/// @file vmexit_probe.cpp
#include "vjl/vmexit_probe.hpp"
#include "vjl/cpuid_detect.hpp"
#include "vjl/timer.hpp"

namespace vjl {

VmexitResult measure_cpuid_cost(std::size_t samples, std::uint64_t tsc_hz) noexcept {
    VmexitResult result{"cpuid", {}, tsc_hz};
    result.samples.reserve(samples);

    // Warmup
    for (int i = 0; i < 1000; ++i) {
        cpuid(0, 0);
    }

    for (std::size_t i = 0; i < samples; ++i) {
        auto before = rdtscp();
        cpuid(0, 0);
        auto after = rdtscp();

        VmexitSample s{};
        s.tsc_before = before;
        s.tsc_after = after;
        s.cost_ns = tsc_to_ns(after - before, tsc_hz);
        result.samples.push_back(s);
    }

    return result;
}

VmexitResult measure_clock_gettime_cost(std::size_t samples, std::uint64_t tsc_hz) noexcept {
    VmexitResult result{"clock_gettime", {}, tsc_hz};
    result.samples.reserve(samples);

    // Warmup
    for (int i = 0; i < 1000; ++i) {
        now_ns();
    }

    for (std::size_t i = 0; i < samples; ++i) {
        auto before = rdtscp();
        auto t = now_ns();
        (void)t;
        auto after = rdtscp();

        VmexitSample s{};
        s.tsc_before = before;
        s.tsc_after = after;
        s.cost_ns = tsc_to_ns(after - before, tsc_hz);
        result.samples.push_back(s);
    }

    return result;
}

} // namespace vjl
