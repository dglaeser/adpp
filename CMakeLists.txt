cmake_minimum_required(VERSION 3.18)

if(NOT DEFINED PROJECT_NAME)
  set(ADPP_SUBPROJECT OFF)
else()
  set(ADPP_SUBPROJECT ON)
endif()

project(adpp)

set(CPP_AD_NAMESPACE adpp)
option(ADPP_BUILD_BENCHMARK "Control if benchmarks should be built" OFF)
option(ADPP_BUILD_EXAMPLES "Control if examples should be built" ON)
option(ADPP_BUILD_TESTS "Control if tests should be built" ON)

include(GNUInstallDirs)
add_library(${PROJECT_NAME} INTERFACE)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)
target_include_directories(${PROJECT_NAME}
    INTERFACE $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}>
              $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Alias to be used in test suite
add_library(${CPP_AD_NAMESPACE}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (NOT ADPP_SUBPROJECT)
    # TODO: installation instructions

    if (ADPP_BUILD_BENCHMARK)
        add_subdirectory(benchmark)
    endif ()

    if (ADPP_BUILD_EXAMPLES)
      enable_testing()
      add_subdirectory(test)
    endif ()
    if (ADPP_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif ()
endif ()
