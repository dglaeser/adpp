#include <cstdlib>

#include <boost/ut.hpp>

#include <cppad/backward/var.hpp>
#include <cppad/backward/derivatives.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

constexpr auto make_derivatives(const auto& a, const auto& b) {
    static_assert(!std::same_as<decltype(a), decltype(b)>);
    cppad::backward::derivatives derivatives{double{}, a, b};
    derivatives[a] = 2.0;
    derivatives[b] = 42.0;
    return derivatives;
}

constexpr void check_constexpr() {
    static constexpr cppad::backward::var<int> a;
    static constexpr cppad::backward::var<int> b;
    static_assert(!std::is_same_v<decltype(a), decltype(b)>);
    constexpr auto derivatives = make_derivatives(a, b);
    static_assert(derivatives[a] == 2.0);
    static_assert(derivatives[b] == 42.0);
}

int main() {
    "bw_derivative_storage"_test = [] () {
        check_constexpr();
        cppad::backward::var a = 1;
        cppad::backward::var b = 1;
        cppad::backward::derivatives derivatives{double{}, a, b};
        derivatives[a] = 2.0;
        derivatives[b] = 5.0;
        expect(eq(derivatives[a], 2.0));
        expect(eq(derivatives[b], 5.0));
    };

    "bw_derivative_computation"_test = [] () {
        cppad::backward::var a = 1;
        cppad::backward::var b = 3;
        cppad::backward::expression e = std::exp(a + b)*(b - 1);
        const auto derivatives = derivatives_of(e, wrt(a, b));
        const auto [value, bp_derivatives] = e.back_propagate(a, b);
        expect(eq(derivatives[a], std::exp(1 + 3)*(3 - 1)));
        expect(eq(derivatives[b], std::exp(1 + 3)*3));
        expect(eq(bp_derivatives[a], std::exp(1 + 3)*(3 - 1)));
        expect(eq(bp_derivatives[b], std::exp(1 + 3)*3));
        expect(eq(value, std::exp(1 + 3)*(3 - 1)));

        expect(eq(derivative_of(e, wrt(a)), std::exp(1 + 3)*(3 - 1)));
        expect(eq(derivative_of(e, wrt(b)), std::exp(1 + 3)*3));
    };

    "bw_higher_order_derivative_computation"_test = [] () {
        int av = 3;
        int bv = 2;
        int cv = 2;
        cppad::backward::var a = av;
        cppad::backward::var b = bv;
        cppad::backward::let c = cv;
        auto expression = a*a*b*c;
        expect(eq(derivative_of(expression, wrt(a), cppad::order::first), 2*av*bv*cv));
        expect(eq(derivative_of(expression, wrt(a), cppad::order::second), 2*bv*cv));
        expect(eq(derivative_of(expression, wrt(a), cppad::order::third), 0));
    };

    return EXIT_SUCCESS;
}
