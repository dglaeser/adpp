#include <iostream>
#include <stdexcept>

#include <cppad/cppad.hpp>

using cppad::var;

int main() {
    auto x = var(2);
    auto y = var(10);
    auto e = (x*3 + y*8)*y;

    std::cout << "e = " << e.value() << std::endl;
    std::cout << "∂e/∂x = " << e.partial(x) << std::endl;
    std::cout << "∂e/∂y = " << e.partial(y) << std::endl;

    if (e.value() != (6 + 80)*10)
        throw std::runtime_error("e.value() is incorrect");
    if (e.partial(x) != 3*10)
        throw std::runtime_error("e.partial(x) is incorrect");
    if (e.partial(y) != (6 + 80) + 80)
        throw std::runtime_error("e.partial(y) is incorrect");

    std::cout << "All checks passed" << std::endl;
}
