#include <iostream>
#include <stdexcept>

#include <cppad/backward.hpp>

using cppad::backward::var;

int main() {
    var x = 2;
    var y = 10;
    const auto e = (x*3 + y*8)*y;
    const auto de_dx = derivative_of(e, wrt(x));
    const auto de_dy = derivative_of(e, wrt(y));

    std::cout << "e = " << e.value() << std::endl;
    std::cout << "∂e/∂x = " << de_dx << std::endl;
    std::cout << "∂e/∂y = " << de_dy << std::endl;

    if (e.value() != (6 + 80)*10)
        throw std::runtime_error("e.value() is incorrect");
    if (de_dx != 3*10)
        throw std::runtime_error("∂e/∂x is incorrect");
    if (de_dy != (6 + 80) + 80)
        throw std::runtime_error("∂e/∂y is incorrect");

    std::cout << "All checks passed" << std::endl;
}
