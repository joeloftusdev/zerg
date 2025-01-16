#include "../include/cpp_logger/global_logger.hpp"
#include <benchmark/benchmark.h>
#include <string>

static void log_benchmark(benchmark::State& state) {
    cpp_logger::setLogFilePath("/dev/null");
    auto logger = cpp_logger::getGlobalLogger();
    
    for (auto _ : state) {
        logger->log(cpp_logger::Verbosity::INFO_LVL, __FILE__, __LINE__, "Test log message");
    }
}

static void log_with_sync_benchmark(benchmark::State& state) {
    cpp_logger::setLogFilePath("/dev/null");
    auto logger = cpp_logger::getGlobalLogger();

    for (auto _ : state) {
        logger->log(cpp_logger::Verbosity::INFO_LVL, __FILE__, __LINE__, "Test log message");
    }

    // Ensure logs are flushed *only once* at the end
    state.PauseTiming();
    logger->sync();
    state.ResumeTiming();
}

// Register benchmarks
BENCHMARK(log_benchmark);
BENCHMARK(log_with_sync_benchmark);


