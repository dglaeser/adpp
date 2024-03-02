cmake_minimum_required(VERSION 3.18)

project(adpp)

set(CPP_AD_NAMESPACE adpp)
option(ADPP_BUILD_BENCHMARKS "Control whether or not to build the benchmarks" OFF)
option(ADPP_BUILD_EXAMPLES "Control whether or not to build the examples" ON)
option(ADPP_BUILD_TESTS "Control whether or not to build the tests" ON)

include(GNUInstallDirs)
add_library(${PROJECT_NAME} INTERFACE)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)
target_include_directories(${PROJECT_NAME}
    INTERFACE $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}>
              $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Alias to be used in test suite
add_library(${CPP_AD_NAMESPACE}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# TODO: installation instructions


if (ADPP_BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif ()

enable_testing()
if (ADPP_BUILD_TESTS)
    add_subdirectory(test)
endif ()
if (ADPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()
