#include <iostream>
#include <utility>

#include <cppad/backward/var.hpp>

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

autodiff::var make_var(auto&& v) { return std::move(v); }
autodiff::var result_of(auto&& expression) { return std::move(expression); }
double as_double(const auto& expression) { return double(expression); }

template<typename E, typename X>
double derivative_of(E&& expression, X&& x) {
    auto [de_dx] = derivatives(std::forward<E>(expression), wrt(std::forward<X>(x)));
    return de_dx;
}
#else
auto make_var(auto&& v) { return cppad::backward::var(std::move(v)); }
auto result_of(auto&& expression) { return std::move(expression); }
double as_double(const auto& expression) { return expression.value(); }

template<typename E, typename X>
double derivative_of(E&& expression, X&& x) {
    return cppad::backward::derivative_of(expression, wrt(x));
}
#endif

int main(int argc, char** argv) {
    auto x = make_var(2.0);
    auto y = make_var(4.0);

    constexpr std::size_t N = 10000;
    std::array<double, N> values;
    std::array<double, N> derivs;
    for (unsigned int i = 0; i < N; ++i) {
        auto r = result_of(GENERATE_EXPRESSION(x, y));
        auto dr_dx = derivative_of(r, x);

        values[i] = as_double(r);
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
