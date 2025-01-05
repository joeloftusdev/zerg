enable_testing()

set(CTEST_OUTPUT_ON_FAILURE ON)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

file(GLOB_RECURSE TestFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h")
file(GLOB_RECURSE SrcFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE IncludeFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/cpp_logger/*.hpp")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests PREFIX "tests" FILES ${TestFiles})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "src" FILES ${SrcFiles})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include PREFIX "include" FILES ${IncludeFiles})

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(gtests ${TestFiles})

target_compile_features(gtests PRIVATE cxx_std_20)

target_include_directories(gtests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(gtests PRIVATE -mavx -msse4.2)

if(SANITIZER_FLAGS)
    target_compile_options(gtests PRIVATE ${SANITIZER_FLAGS})
    target_link_options(gtests PRIVATE ${SANITIZER_FLAGS})
endif()

target_link_libraries(gtests PRIVATE
    cpp_logger
    gtest
    gtest_main
    gmock
    gmock_main
)

include(GoogleTest)
gtest_discover_tests(gtests)