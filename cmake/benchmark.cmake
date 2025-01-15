include(FetchContent)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.6.1
)
FetchContent_MakeAvailable(benchmark)

# Create benchmark executable
file(GLOB_RECURSE BenchmarkFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/benchmark/*.cpp")
add_executable(benchmarks ${BenchmarkFiles})
target_link_libraries(benchmarks PRIVATE benchmark::benchmark cpp_logger)