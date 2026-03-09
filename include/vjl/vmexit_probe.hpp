#pragma once
/// @file vmexit_probe.hpp
/// Probe VM exit costs by executing instructions that typically cause VM exits.
/// CPUID is the canonical example — always traps to the hypervisor.

#include <cstdint>
#include <vector>

namespace vjl {

struct VmexitSample {
    std::uint64_t tsc_before;
    std::uint64_t tsc_after;
    std::int64_t cost_ns;
};

struct VmexitResult {
    const char *instruction; ///< What was measured ("cpuid", "rdmsr_emu", etc.)
    std::vector<VmexitSample> samples;
    std::uint64_t tsc_hz;
};

/// Measure CPUID instruction cost (causes VM exit on all hypervisors).
VmexitResult measure_cpuid_cost(std::size_t samples, std::uint64_t tsc_hz) noexcept;

/// Measure clock_gettime cost (may or may not cause VM exit depending on vDSO).
VmexitResult measure_clock_gettime_cost(std::size_t samples, std::uint64_t tsc_hz) noexcept;

} // namespace vjl
