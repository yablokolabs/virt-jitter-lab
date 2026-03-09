/// @file bench_vmexit.cpp
/// Benchmark VM exit cost across different instructions.
#include "vjl/stats.hpp"
#include "vjl/timer.hpp"
#include "vjl/vmexit_probe.hpp"
#include <cstdio>

int main() {
    auto tsc_hz = vjl::calibrate_tsc_hz();

    std::fprintf(stderr, "VM Exit Cost Benchmark (TSC: %.2f GHz)\n\n",
                 static_cast<double>(tsc_hz) / 1e9);

    // CPUID cost
    {
        auto result = vjl::measure_cpuid_cost(100000, tsc_hz);
        std::vector<std::int64_t> vals;
        for (auto &s : result.samples) vals.push_back(s.cost_ns);
        auto stats = vjl::compute_stats(vals);
        vjl::print_stats(stats, "CPUID instruction", "ns");
    }

    // clock_gettime cost
    {
        auto result = vjl::measure_clock_gettime_cost(100000, tsc_hz);
        std::vector<std::int64_t> vals;
        for (auto &s : result.samples) vals.push_back(s.cost_ns);
        auto stats = vjl::compute_stats(vals);
        vjl::print_stats(stats, "clock_gettime (vDSO)", "ns");
    }

    return 0;
}
