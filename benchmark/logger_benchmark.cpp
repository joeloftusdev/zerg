#include "../include/cpp_logger/global_logger.hpp"
#include <benchmark/benchmark.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <pthread.h>
#if __has_include(<pthread_np.h>)
#include <pthread_np.h>
#endif
#include <sched.h>

namespace
{
void set_thread_attrs(pthread_t thread, int cpu)
{
#if defined(__FreeBSD__)
    cpuset_t cpus;
#else
    cpu_set_t cpus;
#endif
    CPU_ZERO(&cpus);
    CPU_SET(cpu, &cpus);
    if (::pthread_setaffinity_np(thread, sizeof(cpus), &cpus) != 0)
        abort();
}

int getenv_int(const char *name)
{
    const char *env = ::getenv(name);
    if (env == nullptr)
        return -1;
    char *end;
    errno = 0;
    const long result = std::strtol(env, &end, 10);
    if (errno != 0 || name == end || *end != '\0')
    {
        std::cerr << name << "=" << env << " is invalid\n";
        abort();
    }
    return int(result);
}
} // namespace

#define LOG_BENCH(NAME, X, MSGSIZE)                                                                \
    void NAME(benchmark::State &state)                                                             \
    {                                                                                              \
        cpp_logger::setLogFilePath("/dev/null");                                                   \
        auto logger = cpp_logger::getGlobalLogger();                                               \
                                                                                                   \
        if (const int cpu = getenv_int("PRODUCER_CPU"); cpu != -1)                                 \
            set_thread_attrs(::pthread_self(), cpu);                                               \
                                                                                                   \
        std::size_t n = 0;                                                                         \
        constexpr std::size_t sync_every = 1024 * 1024 / (MSGSIZE);                                \
        for (auto _ : state)                                                                       \
        {                                                                                          \
            X;                                                                                     \
            if (++n % sync_every == 0)                                                             \
            {                                                                                      \
                state.PauseTiming();                                                               \
                                                                                                   \
                if (const int cpu = getenv_int("CONSUMER_CPU"); cpu != -1)                         \
                    set_thread_attrs(::pthread_self(), cpu);                                       \
                                                                                                   \
                logger->sync();                                                                    \
                state.ResumeTiming();                                                              \
            }                                                                                      \
        }                                                                                          \
    }                                                                                              \
    BENCHMARK(NAME);

const std::string s{"Hello"};

LOG_BENCH(logger_benchmark,
          logger->log(cpp_logger::Verbosity::INFO_LVL, __FILE__, __LINE__, "Test"), 8)
LOG_BENCH(logger_benchmark_int,
          logger->log(cpp_logger::Verbosity::INFO_LVL, __FILE__, __LINE__, "Test {}", 42), 16)
LOG_BENCH(logger_benchmark_long,
          logger->log(cpp_logger::Verbosity::INFO_LVL, __FILE__, __LINE__, "Test {}", 42L), 16)
LOG_BENCH(logger_benchmark_double,
          logger->log(cpp_logger::Verbosity::INFO_LVL, __FILE__, __LINE__, "Test {}", 42.0), 16)
LOG_BENCH(logger_benchmark_c_str,
          logger->log(cpp_logger::Verbosity::INFO_LVL, __FILE__, __LINE__, "Test {}", "Hello"), 32)
LOG_BENCH(logger_benchmark_str_view,
          logger->log(cpp_logger::Verbosity::INFO_LVL, __FILE__, __LINE__, "Test {}",
                      std::string_view{"Hello"}),
          32)
LOG_BENCH(logger_benchmark_str,
          logger->log(cpp_logger::Verbosity::INFO_LVL, __FILE__, __LINE__, "Test {}", s), 32)

// The above code defines several benchmark tests using the LOG_BENCH macro.
// Each benchmark logs a message with different types of arguments.
// The LOG_BENCH macro sets up the benchmark function, configures the logger, and sets the CPU
// affinity for the producer thread if the PRODUCER_CPU environment variable is set. Additionally,
// it sets the CPU affinity for the consumer thread if the CONSUMER_CPU environment variable is set
// during the pause timing. I.E export PRODUCER_CPU=2 export CONSUMER_CPU=1

// Sample output:
// logger_benchmark                50.9 ns         50.9 ns     13063820
// logger_benchmark_int            61.2 ns         61.2 ns     11383521
// logger_benchmark_long           65.7 ns         65.7 ns     11433033
// logger_benchmark_double          103 ns          103 ns      6903353
// logger_benchmark_c_str          61.3 ns         61.3 ns     10961341
// logger_benchmark_str_view       64.4 ns         64.4 ns      7803286
// logger_benchmark_str            60.8 ns         60.8 ns     11676261