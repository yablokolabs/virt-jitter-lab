cmake_minimum_required(VERSION 3.25)
project(virt-jitter-lab
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "Virtualization timing and IPC latency measurement harness"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
    -Wall -Wextra -Wpedantic -Werror
    -Wno-unused-parameter
)

# ── Library ──────────────────────────────────────────────────

add_library(vjl_core STATIC
    src/timer.cpp
    src/jitter_sampler.cpp
    src/ipc_bench.cpp
    src/vmexit_probe.cpp
    src/cpuid_detect.cpp
    src/stats.cpp
    src/report.cpp
)

target_include_directories(vjl_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(vjl_core PUBLIC pthread rt)

# ── Main executable ──────────────────────────────────────────

add_executable(virt-jitter-lab src/main.cpp)
target_link_libraries(virt-jitter-lab PRIVATE vjl_core)

# ── Tests ────────────────────────────────────────────────────

option(VJL_BUILD_TESTS "Build tests" ON)

if(VJL_BUILD_TESTS)
    enable_testing()

    add_executable(test_timer tests/test_timer.cpp)
    target_link_libraries(test_timer PRIVATE vjl_core)
    add_test(NAME test_timer COMMAND test_timer)

    add_executable(test_jitter tests/test_jitter.cpp)
    target_link_libraries(test_jitter PRIVATE vjl_core)
    add_test(NAME test_jitter COMMAND test_jitter)

    add_executable(test_stats tests/test_stats.cpp)
    target_link_libraries(test_stats PRIVATE vjl_core)
    add_test(NAME test_stats COMMAND test_stats)

    add_executable(test_ipc tests/test_ipc.cpp)
    target_link_libraries(test_ipc PRIVATE vjl_core)
    add_test(NAME test_ipc COMMAND test_ipc)
endif()

# ── Benchmarks ───────────────────────────────────────────────

option(VJL_BUILD_BENCH "Build benchmarks" OFF)

if(VJL_BUILD_BENCH)
    add_executable(bench_vmexit bench/bench_vmexit.cpp)
    target_link_libraries(bench_vmexit PRIVATE vjl_core)
endif()
