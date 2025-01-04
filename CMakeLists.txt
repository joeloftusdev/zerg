cmake_minimum_required(VERSION 3.10)

project(LoggingLibrary)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_library(cpp_logger STATIC src/logger.cpp)

add_executable(main main.cpp)
target_link_libraries(main cpp_logger)

include(gtests)