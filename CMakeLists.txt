cmake_minimum_required(VERSION 3.18)

project(cppad)

set(CPP_AD_NAMESPACE cppad)

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

add_executable(main main.cpp)
target_link_libraries(main PRIVATE cppad::cppad)
