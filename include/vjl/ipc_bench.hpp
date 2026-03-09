#pragma once
/// @file ipc_bench.hpp
/// IPC latency benchmarks: pipes, Unix sockets, shared memory.
/// Measures round-trip latency to characterize virtualization overhead
/// on inter-process communication paths.

#include <cstdint>
#include <vector>

namespace vjl {

enum class IpcMethod : std::uint8_t {
    Pipe,         ///< pipe(2) — kernel-mediated
    UnixSocket,   ///< AF_UNIX socketpair — kernel-mediated
    SharedMemory, ///< POSIX shm + futex — minimal kernel involvement
};

const char *ipc_method_name(IpcMethod m) noexcept;

struct IpcConfig {
    IpcMethod method;
    std::size_t message_size; ///< Bytes per message (1-4096)
    std::size_t round_trips;  ///< Number of ping-pong round trips
    std::size_t warmup;       ///< Warmup round trips (discarded)
};

struct IpcSample {
    std::int64_t round_trip_ns;
    std::uint64_t tsc_start;
    std::uint64_t tsc_end;
};

struct IpcResult {
    IpcMethod method;
    std::size_t message_size;
    std::vector<IpcSample> samples;
    std::uint64_t tsc_hz;
};

/// Run IPC latency benchmark. Forks a child process internally.
IpcResult measure_ipc(const IpcConfig &config) noexcept;

} // namespace vjl
