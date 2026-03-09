#pragma once
/// @file cpuid_detect.hpp
/// CPUID-based feature detection (hypervisor, TSC invariant, etc.)

#include <cstdint>

namespace vjl {

struct CpuidResult {
    std::uint32_t eax, ebx, ecx, edx;
};

/// Execute CPUID instruction.
CpuidResult cpuid(std::uint32_t leaf, std::uint32_t subleaf = 0) noexcept;

/// Check if hypervisor present bit is set (CPUID.01H:ECX bit 31).
bool cpuid_hypervisor_present() noexcept;

/// Get hypervisor vendor string from CPUID leaf 0x40000000.
/// Returns empty string if no hypervisor.
const char *cpuid_hypervisor_vendor() noexcept;

/// Check if invariant TSC is supported (CPUID.80000007H:EDX bit 8).
bool cpuid_invariant_tsc() noexcept;

/// Get processor brand string.
const char *cpuid_brand_string() noexcept;

/// Get number of online CPUs.
int online_cpus() noexcept;

} // namespace vjl
