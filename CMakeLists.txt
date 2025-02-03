cmake_minimum_required(VERSION 3.10)

project(LoggingLibrary)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -Wshadow")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcx16")
endif()


include_directories(include)
include_directories(external/fmt/include)

add_subdirectory(external/fmt)

add_library(cpp_logger STATIC src/logger.cpp)
target_link_libraries(cpp_logger fmt::fmt)

add_executable(main main.cpp)
target_link_libraries(main cpp_logger fmt::fmt)


add_custom_target(assembly
    COMMAND ${CMAKE_CXX_COMPILER} 
            -E 
            -I${CMAKE_SOURCE_DIR}/include
            -std=c++17
            ${CMAKE_SOURCE_DIR}/src/lock_free_queue.cpp > lock_free_queue.ii &&
    ${CMAKE_CXX_COMPILER} -S 
            -masm=intel
            -fverbose-asm
            -O3
            -ffreestanding
            -fno-asynchronous-unwind-tables
            -fcf-protection=none
            -fno-dwarf2-cfi-asm
            -fomit-frame-pointer
            -fno-exceptions
            -fno-rtti
            -fno-unroll-loops
            -std=c++17
            lock_free_queue.ii
            -o ${CMAKE_BINARY_DIR}/lock_free_queue.asm
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating readable assembly for lock_free_queue.cpp"
)

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
    add_definitions(-DBENCHMARK_MODE)
    include(benchmark)
endif()