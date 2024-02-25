include(FetchContent)
FetchContent_Declare(
  ut
  GIT_REPOSITORY https://github.com/boost-ext/ut
  GIT_TAG v1.1.9
)
FetchContent_MakeAvailable(ut)

function (cppad_add_test NAME SOURCE)
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME} PRIVATE cppad::cppad Boost::ut)
    add_test(NAME ${NAME} COMMAND ./${NAME})
endfunction ()

add_subdirectory(backward)

cppad_add_test(test_common test_common.cpp)
