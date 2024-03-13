#include <stdexcept>
#include <iostream>
#include <utility>
#include <array>

#include <adpp/backward/symbols.hpp>
#include <adpp/backward/operators.hpp>
#include <adpp/backward/evaluate.hpp>
#include <adpp/backward/differentiate.hpp>

#include "test_expr.hpp"

int main(int argc, char** argv) {
    if (argc < 3)
        throw std::runtime_error("Expected two input arguments (x, y)");

    double xv = std::atof(argv[1]);
    double yv = std::atof(argv[2]);
    adpp::backward::var x;
    adpp::backward::var y;

    constexpr std::size_t N = 10000;
    double value = 0.0;
    std::array derivs{0.0, 0.0};
    for (unsigned int i = 0; i < N; ++i) {
        const auto expression = GENERATE_EXPRESSION(x, y);
        const auto eval = expression.evaluate(at(x = xv, y = yv));
        const auto gradient = grad(expression, at(x = xv, y = yv));

        value += eval;
        derivs[0] += gradient[x];
        derivs[1] += gradient[y];
    }

    value /= N;
    derivs[0] /= N;
    derivs[1] /= N;

    std::cout << "f(x, y) = " << value << std::endl;
    std::cout << "∂r/∂x = " << derivs[0] << std::endl;
    std::cout << "∂r/∂y = " << derivs[1] << std::endl;

    return 0;
}
