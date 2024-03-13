#include <iostream>
#include <stdexcept>

#include <adpp/backward.hpp>

using adpp::backward::var;

template<adpp::scalar T>
void check(const T& value, const T& expected, const std::string& name) {
    if (value != expected)
        throw std::runtime_error(
            name + " is incorrect: " + std::to_string(value)
            + " (expected " + std::to_string(expected) + ")"
        );
}

int main() {
    var x;
    var y;
    const auto e = (x*3 + y*8)*y;
    const auto de_dx = derivative_of(e, wrt(x), at(x = 2, y = 10));
    const auto de_dy = derivative_of(e, wrt(y), at(x = 2, y = 10));
    const auto value = evaluate(e, at(x = 2, y = 10));
    std::cout << "e = " << value << std::endl;
    std::cout << "∂e/∂x = " << de_dx << std::endl;
    std::cout << "∂e/∂y = " << de_dy << std::endl;

    check(value, (2*3 + 10*8)*10, "value");
    check(de_dx, 3*10, "∂e/∂x");
    check(de_dy, (6 + 80) + 80, "∂e/∂y");
    std::cout << "All checks passed" << std::endl;
}
