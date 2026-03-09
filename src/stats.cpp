/// @file stats.cpp
#include "vjl/stats.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace vjl {

Stats compute_stats(std::vector<std::int64_t> values) noexcept {
    Stats s{};
    if (values.empty()) return s;

    std::sort(values.begin(), values.end());
    s.count = values.size();
    s.min = values.front();
    s.max = values.back();

    double sum = 0;
    for (auto v : values) sum += static_cast<double>(v);
    s.mean = sum / static_cast<double>(s.count);

    double var = 0;
    for (auto v : values) {
        double d = static_cast<double>(v) - s.mean;
        var += d * d;
    }
    s.stddev = std::sqrt(var / static_cast<double>(s.count));

    auto pct = [&](double p) -> std::int64_t {
        auto idx = static_cast<std::size_t>(static_cast<double>(s.count) * p);
        if (idx >= s.count) idx = s.count - 1;
        return values[idx];
    };

    s.median = pct(0.50);
    s.p90 = pct(0.90);
    s.p95 = pct(0.95);
    s.p99 = pct(0.99);
    s.p999 = pct(0.999);
    s.p9999 = pct(0.9999);

    // Count outliers (> mean + 3σ)
    double threshold = s.mean + 3.0 * s.stddev;
    s.outliers = 0;
    for (auto v : values) {
        if (static_cast<double>(v) > threshold) ++s.outliers;
    }

    return s;
}

void print_stats(const Stats &s, const char *label, const char *unit) noexcept {
    std::fprintf(stderr,
                 "┌──────────────────────────────────────────────┐\n"
                 "│  %-42s  │\n"
                 "├──────────────────────────────────────────────┤\n"
                 "│  Samples:    %-10zu                       │\n"
                 "├──────────────────────────────────────────────┤\n"
                 "│  Min:        %-10ld %s                   │\n"
                 "│  Mean:       %-10.0f %s (σ=%.0f)          │\n"
                 "│  Median:     %-10ld %s                   │\n"
                 "│  p90:        %-10ld %s                   │\n"
                 "│  p95:        %-10ld %s                   │\n"
                 "│  p99:        %-10ld %s                   │\n"
                 "│  p99.9:      %-10ld %s                   │\n"
                 "│  p99.99:     %-10ld %s                   │\n"
                 "│  Max:        %-10ld %s                   │\n"
                 "│  Outliers:   %-10zu (>3σ)                 │\n"
                 "└──────────────────────────────────────────────┘\n",
                 label, s.count, s.min, unit, s.mean, unit, s.stddev, s.median, unit, s.p90, unit,
                 s.p95, unit, s.p99, unit, s.p999, unit, s.p9999, unit, s.max, unit, s.outliers);
}

void print_histogram(const std::vector<std::int64_t> &values, int bins, const char *unit) noexcept {
    if (values.empty()) return;

    auto sorted = values;
    std::sort(sorted.begin(), sorted.end());
    auto lo = sorted.front();
    auto hi = sorted.back();
    if (lo == hi) {
        std::fprintf(stderr, "All samples: %ld %s\n", lo, unit);
        return;
    }

    double step = static_cast<double>(hi - lo) / bins;
    std::vector<int> counts(bins, 0);
    for (auto v : sorted) {
        int idx = static_cast<int>(static_cast<double>(v - lo) / step);
        if (idx >= bins) idx = bins - 1;
        counts[idx]++;
    }

    int max_count = *std::max_element(counts.begin(), counts.end());
    int bar_width = 40;

    std::fprintf(stderr, "\nHistogram (%zu samples):\n", values.size());
    for (int i = 0; i < bins; ++i) {
        auto bucket_lo = lo + static_cast<std::int64_t>(i * step);
        int bar_len = max_count > 0 ? (counts[i] * bar_width / max_count) : 0;
        std::fprintf(stderr, "  %8ld %s │", bucket_lo, unit);
        for (int j = 0; j < bar_len; ++j) std::fprintf(stderr, "█");
        std::fprintf(stderr, " (%d)\n", counts[i]);
    }
}

} // namespace vjl
