#pragma once
/// @file timer.hpp
/// High-resolution timing primitives using CLOCK_MONOTONIC and TSC.
/// Designed to measure virtualization-induced timing anomalies.

#include <cstdint>
#include <time.h>

namespace vjl {

/// Read TSC (no serialization — measures start).
inline std::uint64_t rdtsc() noexcept {
    std::uint32_t lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return (static_cast<std::uint64_t>(hi) << 32) | lo;
}

/// Read TSC with serialization (measures end — waits for prior instructions).
inline std::uint64_t rdtscp() noexcept {
    std::uint32_t lo, hi, aux;
    asm volatile("rdtscp" : "=a"(lo), "=d"(hi), "=c"(aux));
    return (static_cast<std::uint64_t>(hi) << 32) | lo;
}

/// CLOCK_MONOTONIC nanoseconds.
inline std::int64_t now_ns() noexcept {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<std::int64_t>(ts.tv_sec) * 1'000'000'000LL + ts.tv_nsec;
}

/// Calibrate TSC frequency (Hz) by comparing TSC delta to CLOCK_MONOTONIC.
/// Takes ~100ms to complete.
std::uint64_t calibrate_tsc_hz() noexcept;

/// Convert TSC ticks to nanoseconds given a known frequency.
inline std::int64_t tsc_to_ns(std::uint64_t ticks, std::uint64_t tsc_hz) noexcept {
    return static_cast<std::int64_t>((ticks * 1'000'000'000ULL) / tsc_hz);
}

/// Detect if running inside a virtual machine.
bool detect_hypervisor() noexcept;

/// Get hypervisor vendor string (or "bare-metal" if none).
const char *hypervisor_vendor() noexcept;

} // namespace vjl
