/// @file main.cpp
/// virt-jitter-lab: comprehensive virtualization timing assessment.
///
/// Usage:
///   virt-jitter-lab [--samples N] [--sleep-us U] [--cpu C] [--ipc METHOD]

#include "vjl/cpuid_detect.hpp"
#include "vjl/ipc_bench.hpp"
#include "vjl/jitter_sampler.hpp"
#include "vjl/report.hpp"
#include "vjl/stats.hpp"
#include "vjl/timer.hpp"
#include "vjl/vmexit_probe.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char *argv[]) {
    std::size_t samples = 10000;
    int sleep_us = 100;
    int cpu_pin = -1;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--samples") == 0 && i + 1 < argc)
            samples = static_cast<std::size_t>(std::atol(argv[++i]));
        else if (std::strcmp(argv[i], "--sleep-us") == 0 && i + 1 < argc)
            sleep_us = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--cpu") == 0 && i + 1 < argc)
            cpu_pin = std::atoi(argv[++i]);
    }

    std::fprintf(stderr, "╔══════════════════════════════════════════════╗\n"
                         "║        virt-jitter-lab v0.1.0                ║\n"
                         "║  Virtualization timing assessment toolkit    ║\n"
                         "╚══════════════════════════════════════════════╝\n\n");

    auto env = vjl::detect_environment();

    std::fprintf(stderr,
                 "Config:\n"
                 "  Samples:     %zu\n"
                 "  Sleep:       %d μs\n"
                 "  CPU pin:     %s\n"
                 "  Virtualized: %s (%s)\n\n",
                 samples, sleep_us, cpu_pin >= 0 ? "yes" : "no", env.is_virtualized ? "YES" : "NO",
                 env.hypervisor);

    // 1. Timer jitter
    std::fprintf(stderr, "Measuring timer jitter (%zu × %dμs sleeps)...\n", samples, sleep_us);
    vjl::JitterConfig jcfg{sleep_us, samples, false, cpu_pin};
    auto jitter = vjl::measure_jitter(jcfg);

    // 2. Preemption gaps (fewer samples — tight loop is fast)
    std::fprintf(stderr, "Measuring scheduling preemption (tight loop)...\n");
    auto preemption = vjl::measure_preemption(samples, cpu_pin);

    // 3. IPC latency (pipe)
    std::fprintf(stderr, "Measuring IPC latency (pipe, 64 bytes)...\n");
    vjl::IpcConfig ipc_cfg{vjl::IpcMethod::Pipe, 64, samples / 10, 100};
    auto ipc_pipe = vjl::measure_ipc(ipc_cfg);

    // 4. VM exit cost
    std::fprintf(stderr, "Measuring VM exit cost (CPUID)...\n");
    auto cpuid_cost = vjl::measure_cpuid_cost(samples, env.tsc_hz);

    // Full report
    vjl::print_report(env, jitter, preemption, ipc_pipe, cpuid_cost);

    return 0;
}
