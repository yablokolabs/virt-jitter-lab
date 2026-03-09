/// @file ipc_bench.cpp
#include "vjl/ipc_bench.hpp"
#include "vjl/timer.hpp"
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <linux/futex.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

namespace vjl {

const char *ipc_method_name(IpcMethod m) noexcept {
    switch (m) {
    case IpcMethod::Pipe:
        return "pipe";
    case IpcMethod::UnixSocket:
        return "unix_socket";
    case IpcMethod::SharedMemory:
        return "shared_memory";
    }
    return "unknown";
}

static int futex_wait(std::atomic<int> *addr, int val) {
    return static_cast<int>(syscall(SYS_futex, addr, FUTEX_WAIT, val, nullptr, nullptr, 0));
}

static int futex_wake(std::atomic<int> *addr) {
    return static_cast<int>(syscall(SYS_futex, addr, FUTEX_WAKE, 1, nullptr, nullptr, 0));
}

static IpcResult bench_pipe(const IpcConfig &config, std::uint64_t tsc_hz) {
    IpcResult result{IpcMethod::Pipe, config.message_size, {}, tsc_hz};

    int p2c[2], c2p[2];
    if (pipe(p2c) != 0 || pipe(c2p) != 0) return result;

    auto buf = std::vector<char>(config.message_size, 'X');

    pid_t pid = fork();
    if (pid == 0) {
        // Child: echo back
        close(p2c[1]);
        close(c2p[0]);
        for (std::size_t i = 0; i < config.warmup + config.round_trips; ++i) {
            auto n = read(p2c[0], buf.data(), config.message_size);
            (void)n;
            auto w = write(c2p[1], buf.data(), config.message_size);
            (void)w;
        }
        close(p2c[0]);
        close(c2p[1]);
        _exit(0);
    }

    // Parent: ping-pong
    close(p2c[0]);
    close(c2p[1]);

    result.samples.reserve(config.round_trips);

    for (std::size_t i = 0; i < config.warmup + config.round_trips; ++i) {
        auto tsc0 = rdtsc();
        auto t0 = now_ns();

        auto w = write(p2c[1], buf.data(), config.message_size);
        (void)w;
        auto n = read(c2p[0], buf.data(), config.message_size);
        (void)n;

        auto tsc1 = rdtscp();
        auto t1 = now_ns();

        if (i >= config.warmup) {
            result.samples.push_back({t1 - t0, tsc0, tsc1});
        }
    }

    close(p2c[1]);
    close(c2p[0]);
    waitpid(pid, nullptr, 0);
    return result;
}

static IpcResult bench_unix_socket(const IpcConfig &config, std::uint64_t tsc_hz) {
    IpcResult result{IpcMethod::UnixSocket, config.message_size, {}, tsc_hz};

    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    auto buf = std::vector<char>(config.message_size, 'Y');

    pid_t pid = fork();
    if (pid == 0) {
        close(fds[0]);
        for (std::size_t i = 0; i < config.warmup + config.round_trips; ++i) {
            auto n = read(fds[1], buf.data(), config.message_size);
            (void)n;
            auto w = write(fds[1], buf.data(), config.message_size);
            (void)w;
        }
        close(fds[1]);
        _exit(0);
    }

    close(fds[1]);
    result.samples.reserve(config.round_trips);

    for (std::size_t i = 0; i < config.warmup + config.round_trips; ++i) {
        auto tsc0 = rdtsc();
        auto t0 = now_ns();

        auto w = write(fds[0], buf.data(), config.message_size);
        (void)w;
        auto n = read(fds[0], buf.data(), config.message_size);
        (void)n;

        auto tsc1 = rdtscp();
        auto t1 = now_ns();

        if (i >= config.warmup) {
            result.samples.push_back({t1 - t0, tsc0, tsc1});
        }
    }

    close(fds[0]);
    waitpid(pid, nullptr, 0);
    return result;
}

static IpcResult bench_shm(const IpcConfig &config, std::uint64_t tsc_hz) {
    IpcResult result{IpcMethod::SharedMemory, config.message_size, {}, tsc_hz};

    // Shared memory region: [futex_parent(4)] [futex_child(4)] [data(msg_size)]
    std::size_t shm_size = 8 + config.message_size;
    auto *mem = static_cast<char *>(
        mmap(nullptr, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));

    auto *flag_parent = reinterpret_cast<std::atomic<int> *>(mem);
    auto *flag_child = reinterpret_cast<std::atomic<int> *>(mem + 4);
    char *data = mem + 8;

    flag_parent->store(0);
    flag_child->store(0);

    pid_t pid = fork();
    if (pid == 0) {
        for (std::size_t i = 0; i < config.warmup + config.round_trips; ++i) {
            // Wait for parent signal
            while (flag_parent->load() == 0) {
                futex_wait(flag_parent, 0);
            }
            flag_parent->store(0);

            // "Process" and signal back
            flag_child->store(1);
            futex_wake(flag_child);
        }
        _exit(0);
    }

    result.samples.reserve(config.round_trips);
    std::memset(data, 'Z', config.message_size);

    for (std::size_t i = 0; i < config.warmup + config.round_trips; ++i) {
        auto tsc0 = rdtsc();
        auto t0 = now_ns();

        // Signal child
        flag_parent->store(1);
        futex_wake(flag_parent);

        // Wait for child response
        while (flag_child->load() == 0) {
            futex_wait(flag_child, 0);
        }
        flag_child->store(0);

        auto tsc1 = rdtscp();
        auto t1 = now_ns();

        if (i >= config.warmup) {
            result.samples.push_back({t1 - t0, tsc0, tsc1});
        }
    }

    waitpid(pid, nullptr, 0);
    munmap(mem, shm_size);
    return result;
}

IpcResult measure_ipc(const IpcConfig &config) noexcept {
    auto tsc_hz = calibrate_tsc_hz();
    switch (config.method) {
    case IpcMethod::Pipe:
        return bench_pipe(config, tsc_hz);
    case IpcMethod::UnixSocket:
        return bench_unix_socket(config, tsc_hz);
    case IpcMethod::SharedMemory:
        return bench_shm(config, tsc_hz);
    }
    return {config.method, config.message_size, {}, tsc_hz};
}

} // namespace vjl
