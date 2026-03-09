/// @file test_ipc.cpp
#include "vjl/ipc_bench.hpp"
#include <cstdio>

void test_pipe_roundtrip() {
    vjl::IpcConfig cfg{vjl::IpcMethod::Pipe, 64, 50, 10};
    auto result = vjl::measure_ipc(cfg);

    if (result.samples.size() != 50) {
        std::fprintf(stderr, "  ✗ pipe: expected 50 samples, got %zu\n", result.samples.size());
        return;
    }

    // All round trips should be positive and < 10ms
    for (auto &s : result.samples) {
        if (s.round_trip_ns <= 0 || s.round_trip_ns > 10'000'000) {
            std::fprintf(stderr, "  ✗ pipe: suspicious round trip: %ld ns\n", s.round_trip_ns);
            return;
        }
    }

    std::fprintf(stderr, "  ✓ pipe IPC (50 round trips)\n");
}

void test_unix_socket_roundtrip() {
    vjl::IpcConfig cfg{vjl::IpcMethod::UnixSocket, 64, 50, 10};
    auto result = vjl::measure_ipc(cfg);

    if (result.samples.size() != 50) {
        std::fprintf(stderr, "  ✗ unix socket: expected 50, got %zu\n", result.samples.size());
        return;
    }
    std::fprintf(stderr, "  ✓ unix socket IPC (50 round trips)\n");
}

void test_shm_roundtrip() {
    vjl::IpcConfig cfg{vjl::IpcMethod::SharedMemory, 64, 50, 10};
    auto result = vjl::measure_ipc(cfg);

    if (result.samples.size() != 50) {
        std::fprintf(stderr, "  ✗ shm: expected 50, got %zu\n", result.samples.size());
        return;
    }
    std::fprintf(stderr, "  ✓ shared memory IPC (50 round trips)\n");
}

int main() {
    std::fprintf(stderr, "test_ipc:\n");
    test_pipe_roundtrip();
    test_unix_socket_roundtrip();
    test_shm_roundtrip();
    std::fprintf(stderr, "  All IPC tests passed.\n");
    return 0;
}
