#include <iostream>

#include "cppad.hpp"

using cppad::var;

int main() {
    auto a = var(3.0);
    auto b = var(2.0);
    auto plus = a + b;
    auto times = a * var(42.0);
    std::cout << "plus (a + b) = " << plus.value() << std::endl;
    std::cout << "times (a * 42) = " << times.value() << std::endl;

    std::cout << "∂plus/∂a = " << plus.partial(a) << std::endl;
    std::cout << "∂times/∂a = " << times.partial(a) << std::endl;
}
