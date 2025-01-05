cmake_minimum_required(VERSION 3.10)

project(LoggingLibrary)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_library(cpp_logger STATIC src/logger.cpp)

add_executable(main main.cpp)
target_link_libraries(main cpp_logger)

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

include(gtests)
