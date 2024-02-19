function (cppad_add_example NAME SOURCE)
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME} cppad::cppad)
endfunction ()

cppad_add_example(add_and_multiply add_and_multiply.cpp)
