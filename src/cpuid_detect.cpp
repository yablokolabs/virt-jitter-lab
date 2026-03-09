/// @file cpuid_detect.cpp
#include "vjl/cpuid_detect.hpp"
#include <cstring>
#include <unistd.h>

namespace vjl {

CpuidResult cpuid(std::uint32_t leaf, std::uint32_t subleaf) noexcept {
    CpuidResult r{};
    asm volatile("cpuid"
                 : "=a"(r.eax), "=b"(r.ebx), "=c"(r.ecx), "=d"(r.edx)
                 : "a"(leaf), "c"(subleaf));
    return r;
}

bool cpuid_hypervisor_present() noexcept {
    auto r = cpuid(1, 0);
    return (r.ecx >> 31) & 1;
}

const char *cpuid_hypervisor_vendor() noexcept {
    static char vendor[13] = {};
    static bool done = false;
    if (done) return vendor;

    auto r = cpuid(0x40000000, 0);
    std::memcpy(&vendor[0], &r.ebx, 4);
    std::memcpy(&vendor[4], &r.ecx, 4);
    std::memcpy(&vendor[8], &r.edx, 4);
    vendor[12] = '\0';
    done = true;
    return vendor;
}

bool cpuid_invariant_tsc() noexcept {
    auto r = cpuid(0x80000007, 0);
    return (r.edx >> 8) & 1;
}

const char *cpuid_brand_string() noexcept {
    static char brand[49] = {};
    static bool done = false;
    if (done) return brand;

    for (std::uint32_t i = 0; i < 3; ++i) {
        auto r = cpuid(0x80000002 + i, 0);
        std::memcpy(&brand[i * 16], &r.eax, 4);
        std::memcpy(&brand[i * 16 + 4], &r.ebx, 4);
        std::memcpy(&brand[i * 16 + 8], &r.ecx, 4);
        std::memcpy(&brand[i * 16 + 12], &r.edx, 4);
    }
    brand[48] = '\0';
    done = true;
    return brand;
}

int online_cpus() noexcept {
    return static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
}

} // namespace vjl
