#include <stdexcept>
#include <iostream>
#include <utility>

#define ADD_2(x) x + x
#define ADD_4(x) ADD_2(x) + ADD_2(x)
#define ADD_8(x) ADD_4(x) + ADD_4(x)
#define ADD_16(x) ADD_8(x) + ADD_8(x)
#define ADD_32(x) ADD_16(x) + ADD_16(x)
#define ADD_64(x) ADD_32(x) + ADD_32(x)

#define UNIT_EXPRESSION(a, b) a*((a + b)*b + (a*b) + b)
#define GENERATE_EXPRESSION(a, b) ADD_64(UNIT_EXPRESSION(a, b))

#if USE_AUTODIFF
#include <autodiff/reverse/var.hpp>

autodiff::var make_var(auto&& v) {
    return {std::move(v)};
}

autodiff::var as_expr(auto&& expression) {
    return {std::move(expression)};
}

double result_of(const auto& expression, const auto&...) {
    return double(expression);
}

template<typename E>
double derivative(E&& expression, const auto& x, const auto& y, double xv, double yv) {
    auto [de_dx] = derivatives(std::forward<E>(expression), wrt(x));
    return de_dx;
}
#else
#include <adpp/backward/symbols.hpp>
#include <adpp/backward/evaluate.hpp>
#include <adpp/backward/differentiate.hpp>

template<auto _ = [] () {}>
auto make_var(auto&&) {
    return adpp::backward::var<double, _>{};
}

auto as_expr(auto&& expression) {
    return std::move(expression);
}

double result_of(const auto& expression, const auto& x, const auto& y, double xv, double yv) {
    return adpp::backward::evaluate(expression, at(x = xv, y = yv));
}

template<typename E>
double derivative(E&& expression, const auto& x, const auto& y, double xv, double yv) {
    return derivative_of(expression, wrt(x), at(x = xv, y = yv));
}
#endif

int main(int argc, char** argv) {
    if (argc < 3)
        throw std::runtime_error("Expected two input arguments (x, y)");

    double xv = std::atof(argv[1]);
    double yv = std::atof(argv[1]);
    auto x = make_var(xv);
    auto y = make_var(yv);

    constexpr std::size_t N = 10000;
    std::array<double, N> values;
    std::array<double, N> derivs;
    for (unsigned int i = 0; i < N; ++i) {
        auto expression = as_expr(GENERATE_EXPRESSION(x, y));
        auto r = result_of(expression, x, y, xv, yv);
        auto dr_dx = derivative(expression, x, y, xv, yv);

        values[i] = r;
        derivs[i] = dr_dx;
    }

    if (argc > 1) {
        double avg = 0, avg_deriv = 0;
        for (unsigned int i = 0; i < N; ++i) {
            avg += values[i];
            avg_deriv += derivs[i];
        }
        avg /= N;
        avg_deriv /= N;
        std::cout << "r = " << avg << std::endl;
        std::cout << "∂r/∂x = " << avg_deriv << std::endl;
    }
    return 0;
}
