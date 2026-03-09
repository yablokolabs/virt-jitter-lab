/// @file test_stats.cpp
#include "vjl/stats.hpp"
#include <cmath>
#include <cstdio>

void test_basic_stats() {
    std::vector<std::int64_t> vals = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    auto s = vjl::compute_stats(vals);

    if (s.count != 10) {
        std::fprintf(stderr, "  ✗ count\n");
        return;
    }
    if (s.min != 10) {
        std::fprintf(stderr, "  ✗ min: %ld\n", s.min);
        return;
    }
    if (s.max != 100) {
        std::fprintf(stderr, "  ✗ max: %ld\n", s.max);
        return;
    }
    if (std::fabs(s.mean - 55.0) > 0.01) {
        std::fprintf(stderr, "  ✗ mean: %f\n", s.mean);
        return;
    }
    if (s.median != 60) {
        std::fprintf(stderr, "  ✗ median: %ld\n", s.median);
        return;
    }

    std::fprintf(stderr, "  ✓ basic stats\n");
}

void test_single_value() {
    std::vector<std::int64_t> vals = {42};
    auto s = vjl::compute_stats(vals);

    if (s.count != 1 || s.min != 42 || s.max != 42) {
        std::fprintf(stderr, "  ✗ single value\n");
        return;
    }
    std::fprintf(stderr, "  ✓ single value\n");
}

void test_outlier_detection() {
    std::vector<std::int64_t> vals;
    for (int i = 0; i < 100; ++i) vals.push_back(100);
    vals.push_back(100000); // extreme outlier

    auto s = vjl::compute_stats(vals);
    if (s.outliers < 1) {
        std::fprintf(stderr, "  ✗ outlier not detected\n");
        return;
    }
    std::fprintf(stderr, "  ✓ outlier detection (%zu outliers)\n", s.outliers);
}

int main() {
    std::fprintf(stderr, "test_stats:\n");
    test_basic_stats();
    test_single_value();
    test_outlier_detection();
    std::fprintf(stderr, "  All stats tests passed.\n");
    return 0;
}
