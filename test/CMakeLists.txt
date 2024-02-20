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

cppad_add_test(test_detail test_detail.cpp)
cppad_add_test(test_concepts test_concepts.cpp)
cppad_add_test(test_constant test_constant.cpp)
cppad_add_test(test_variable test_variable.cpp)
cppad_add_test(test_operators test_operators.cpp)
cppad_add_test(test_function test_function.cpp)
