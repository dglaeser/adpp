#include <stdexcept>
#include <iostream>
#include <utility>
#include <chrono>

#if USE_AUTODIFF
#include <autodiff/reverse/var.hpp>
#else
#include <adpp/backward/symbols.hpp>
#include <adpp/backward/expression.hpp>
#endif

#include "test_expr.hpp"

int main(int argc, char** argv) {
    if (argc < 3)
        throw std::runtime_error("Expected two input arguments (x, y)");

    double xv = std::atof(argv[1]);
    double yv = std::atof(argv[2]);

    constexpr std::size_t N = 10000;
    double value = 0.0;
    std::array derivs{0.0, 0.0};
    for (unsigned int i = 0; i < N; ++i) {
#if USE_AUTODIFF
        autodiff::var x;
        autodiff::var y;
        const autodiff::var expression = GENERATE_EXPRESSION(x, y);
        const auto r = double(expression);
        const auto [dr_dx] = derivatives(expression, wrt(x));
        const auto [dr_dy] = derivatives(expression, wrt(y));
#else
        adpp::backward::var<double> x;
        adpp::backward::var<double> y;
        const auto expression = GENERATE_EXPRESSION(x, y);
        const auto r = expression(at(x = xv, y = yv));
        const auto dr_dx = expression.back_propagate(at(x = xv, y = yv), wrt(x)).second[x];
        const auto dr_dy = expression.back_propagate(at(x = xv, y = yv), wrt(y)).second[y];
#endif

        value += r;
        derivs[0] += dr_dx;
        derivs[1] += dr_dy;
    }

    value /= N;
    derivs[0] /= N;
    derivs[1] /= N;

    std::cout << "r = " << value << std::endl;
    std::cout << "∂r/∂x = " << derivs[0] << std::endl;
    std::cout << "∂r/∂y = " << derivs[1] << std::endl;

    return 0;
}
