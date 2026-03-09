/// @file timer.cpp
#include "vjl/timer.hpp"
#include "vjl/cpuid_detect.hpp"
#include <chrono>
#include <thread>

namespace vjl {

std::uint64_t calibrate_tsc_hz() noexcept {
    auto t0 = now_ns();
    auto tsc0 = rdtscp();

    // Sleep ~100ms for calibration
    struct timespec req {
        0, 100'000'000
    };
    clock_nanosleep(CLOCK_MONOTONIC, 0, &req, nullptr);

    auto tsc1 = rdtscp();
    auto t1 = now_ns();

    auto elapsed_ns = t1 - t0;
    auto tsc_delta = tsc1 - tsc0;

    if (elapsed_ns <= 0) return 2'000'000'000ULL; // fallback
    return (tsc_delta * 1'000'000'000ULL) / static_cast<std::uint64_t>(elapsed_ns);
}

bool detect_hypervisor() noexcept {
    return cpuid_hypervisor_present();
}

const char *hypervisor_vendor() noexcept {
    if (!cpuid_hypervisor_present()) return "bare-metal";
    return cpuid_hypervisor_vendor();
}

} // namespace vjl
