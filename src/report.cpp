/// @file report.cpp
#include "vjl/report.hpp"
#include "vjl/cpuid_detect.hpp"
#include <cstdio>
#include <cstring>
#include <sys/utsname.h>

namespace vjl {

EnvironmentInfo detect_environment() noexcept {
    EnvironmentInfo env{};
    env.is_virtualized = cpuid_hypervisor_present();
    env.hypervisor = env.is_virtualized ? cpuid_hypervisor_vendor() : "bare-metal";
    env.tsc_hz = calibrate_tsc_hz();
    env.num_cpus = online_cpus();

    static char kver[256];
    struct utsname u;
    if (uname(&u) == 0) {
        std::snprintf(kver, sizeof(kver), "%s %s", u.sysname, u.release);
    } else {
        std::strncpy(kver, "unknown", sizeof(kver));
    }
    env.kernel_version = kver;

    return env;
}

void print_report(const EnvironmentInfo &env, const JitterResult &jitter,
                  const JitterResult &preemption, const IpcResult &ipc_pipe,
                  const VmexitResult &cpuid_cost) noexcept {
    std::fprintf(stderr, "\n"
                         "╔══════════════════════════════════════════════╗\n"
                         "║     virt-jitter-lab Assessment Report        ║\n"
                         "╚══════════════════════════════════════════════╝\n\n");

    // Environment
    std::fprintf(stderr,
                 "Environment:\n"
                 "  Virtualized:  %s\n"
                 "  Hypervisor:   %s\n"
                 "  CPU:          %s\n"
                 "  Cores:        %d\n"
                 "  TSC freq:     %.2f GHz\n"
                 "  Invariant TSC:%s\n"
                 "  Kernel:       %s\n\n",
                 env.is_virtualized ? "YES" : "NO", env.hypervisor, cpuid_brand_string(),
                 env.num_cpus, static_cast<double>(env.tsc_hz) / 1e9,
                 cpuid_invariant_tsc() ? " yes" : " no", env.kernel_version);

    // Jitter analysis
    {
        std::vector<std::int64_t> vals;
        vals.reserve(jitter.samples.size());
        for (auto &s : jitter.samples) vals.push_back(s.jitter_ns);
        auto stats = compute_stats(std::move(vals));
        std::fprintf(stderr, "── Timer Jitter (sleep-based) ──\n");
        print_stats(stats, "Sleep Jitter", "ns");
    }

    // Preemption analysis
    {
        std::vector<std::int64_t> vals;
        vals.reserve(preemption.samples.size());
        for (auto &s : preemption.samples) vals.push_back(s.jitter_ns);
        auto stats = compute_stats(std::move(vals));
        std::fprintf(stderr, "\n── Scheduling Preemption (tight loop) ──\n");
        print_stats(stats, "Preemption Gaps", "ns");
    }

    // IPC latency
    {
        std::vector<std::int64_t> vals;
        vals.reserve(ipc_pipe.samples.size());
        for (auto &s : ipc_pipe.samples) vals.push_back(s.round_trip_ns);
        auto stats = compute_stats(std::move(vals));
        std::fprintf(stderr, "\n── IPC Latency (%s, %zu bytes) ──\n",
                     ipc_method_name(ipc_pipe.method), ipc_pipe.message_size);
        print_stats(stats, "IPC Round-Trip", "ns");
    }

    // VM exit cost
    {
        std::vector<std::int64_t> vals;
        vals.reserve(cpuid_cost.samples.size());
        for (auto &s : cpuid_cost.samples) vals.push_back(s.cost_ns);
        auto stats = compute_stats(std::move(vals));
        std::fprintf(stderr, "\n── VM Exit Cost (%s) ──\n", cpuid_cost.instruction);
        print_stats(stats, "CPUID VM Exit", "ns");
    }

    // Verdict
    std::fprintf(stderr,
                 "\n══════════════════════════════════════════════\n"
                 "  Assessment: %s environment detected.\n",
                 env.is_virtualized ? "VIRTUALIZED" : "BARE-METAL");

    if (env.is_virtualized) {
        std::fprintf(stderr, "  Hypervisor overhead is present in all measurements.\n"
                             "  Consider bare-metal or RT-tuned hypervisor for\n"
                             "  safety-critical real-time workloads.\n");
    } else {
        std::fprintf(stderr, "  No virtualization overhead detected.\n"
                             "  Timing measurements reflect native hardware.\n");
    }
    std::fprintf(stderr, "══════════════════════════════════════════════\n\n");
}

} // namespace vjl
