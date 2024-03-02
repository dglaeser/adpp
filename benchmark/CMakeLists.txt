# autodiff seems to not handle transitive linking etc :/
find_package(autodiff REQUIRED)
find_package(eigen3 REQUIRED)

function (adpp_add_benchmark NAME SOURCE)
    add_executable(${NAME} ${SOURCE})
    target_compile_options(${NAME} PRIVATE -O3)
    target_include_directories(${NAME} PRIVATE ${EIGEN3_INCLUDE_DIR})
    target_link_libraries(${NAME} adpp::adpp autodiff::autodiff Eigen3::Eigen)
endfunction ()

add_subdirectory(backwards)
