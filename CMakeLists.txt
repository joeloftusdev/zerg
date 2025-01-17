cmake_minimum_required(VERSION 3.10)

project(LoggingLibrary)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()


include_directories(include)
include_directories(external/fmt/include)

add_subdirectory(external/fmt)

add_library(cpp_logger STATIC src/logger.cpp)
target_link_libraries(cpp_logger fmt::fmt)

add_executable(main main.cpp)
target_link_libraries(main cpp_logger fmt::fmt)

option(ENABLE_THREAD_SANITIZER "Enable Thread Sanitizer" OFF)
option(ENABLE_ADDRESS_SANITIZER "Enable Address and Undefined Sanitizers" ON)

if(ENABLE_THREAD_SANITIZER)
    set(SANITIZER_FLAGS "-fsanitize=thread")
elseif(ENABLE_ADDRESS_SANITIZER)
    set(SANITIZER_FLAGS "-fsanitize=address,undefined")
else()
    set(SANITIZER_FLAGS "")
endif()

if(SANITIZER_FLAGS)
    target_compile_options(main PRIVATE ${SANITIZER_FLAGS})
    target_link_options(main PRIVATE ${SANITIZER_FLAGS})
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_CXX_SOURCE_FILES *.cpp *.hpp)
    add_custom_target(
        clang-format
        COMMAND ${CLANG_FORMAT} -i ${ALL_CXX_SOURCE_FILES}
        COMMENT "Running clang-format"
    )
endif()

include(cpplogger_gtests)

if(BUILD_BENCHMARKS)
    include(benchmark)
endif()