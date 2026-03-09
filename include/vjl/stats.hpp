#pragma once
/// @file stats.hpp
/// Statistical analysis: percentiles, histogram, outlier detection.

#include <cstdint>
#include <string>
#include <vector>

namespace vjl {

struct Stats {
    std::int64_t min;
    std::int64_t max;
    double mean;
    double stddev;
    std::int64_t median;
    std::int64_t p90;
    std::int64_t p95;
    std::int64_t p99;
    std::int64_t p999;
    std::int64_t p9999;
    std::size_t count;
    std::size_t outliers; ///< Samples > mean + 3σ
};

/// Compute statistics from a sorted array of values.
Stats compute_stats(std::vector<std::int64_t> values) noexcept;

/// Print stats to stderr in a formatted table.
void print_stats(const Stats &s, const char *label, const char *unit = "ns") noexcept;

/// Print an ASCII histogram to stderr.
void print_histogram(const std::vector<std::int64_t> &values, int bins = 30,
                     const char *unit = "ns") noexcept;

} // namespace vjl
