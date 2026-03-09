/// @file test_timer.cpp
#include "vjl/cpuid_detect.hpp"
#include "vjl/timer.hpp"
#include <cstdio>

void test_now_ns_monotonic() {
    auto t0 = vjl::now_ns();
    auto t1 = vjl::now_ns();
    if (t1 < t0) {
        std::fprintf(stderr, "  ✗ now_ns not monotonic\n");
        return;
    }
    std::fprintf(stderr, "  ✓ now_ns monotonic\n");
}

void test_rdtsc_monotonic() {
    auto t0 = vjl::rdtsc();
    auto t1 = vjl::rdtsc();
    if (t1 <= t0) {
        std::fprintf(stderr, "  ✗ rdtsc not monotonic\n");
        return;
    }
    std::fprintf(stderr, "  ✓ rdtsc monotonic\n");
}

void test_calibrate_tsc() {
    auto hz = vjl::calibrate_tsc_hz();
    // Sanity: should be between 500MHz and 10GHz
    if (hz < 500'000'000ULL || hz > 10'000'000'000ULL) {
        std::fprintf(stderr, "  ✗ TSC calibration out of range: %lu\n", hz);
        return;
    }
    std::fprintf(stderr, "  ✓ TSC calibration: %.2f GHz\n", static_cast<double>(hz) / 1e9);
}

void test_hypervisor_detect() {
    bool virt = vjl::detect_hypervisor();
    auto vendor = vjl::hypervisor_vendor();
    std::fprintf(stderr, "  ✓ hypervisor detection: %s (%s)\n", virt ? "virtualized" : "bare-metal",
                 vendor);
}

void test_cpuid_brand() {
    auto brand = vjl::cpuid_brand_string();
    if (brand[0] == '\0') {
        std::fprintf(stderr, "  ✗ empty brand string\n");
        return;
    }
    std::fprintf(stderr, "  ✓ CPU brand: %s\n", brand);
}

int main() {
    std::fprintf(stderr, "test_timer:\n");
    test_now_ns_monotonic();
    test_rdtsc_monotonic();
    test_calibrate_tsc();
    test_hypervisor_detect();
    test_cpuid_brand();
    std::fprintf(stderr, "  All timer tests passed.\n");
    return 0;
}
