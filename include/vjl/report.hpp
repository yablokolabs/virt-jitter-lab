#pragma once
/// @file report.hpp
/// Generate a virtualization assessment report.

#include "vjl/ipc_bench.hpp"
#include "vjl/jitter_sampler.hpp"
#include "vjl/stats.hpp"
#include "vjl/vmexit_probe.hpp"

namespace vjl {

struct EnvironmentInfo {
    bool is_virtualized;
    const char *hypervisor;
    std::uint64_t tsc_hz;
    int num_cpus;
    const char *kernel_version;
};

/// Detect environment info.
EnvironmentInfo detect_environment() noexcept;

/// Print full assessment report to stderr.
void print_report(const EnvironmentInfo &env, const JitterResult &jitter,
                  const JitterResult &preemption, const IpcResult &ipc_pipe,
                  const VmexitResult &cpuid_cost) noexcept;

} // namespace vjl
