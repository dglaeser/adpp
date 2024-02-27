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
        expect(eq(derivative_of(expression, wrt(a), cppad::first_order), 2*av*bv*cv));
        expect(eq(derivative_of(expression, wrt(a), cppad::second_order), 2*bv*cv));
        expect(eq(derivative_of(expression, wrt(a), cppad::third_order), 0));
    };

    "bw_higher_order_derivative_computation_at_compile_time"_test = [] () {
        static constexpr int av = 3;
        static constexpr int bv = 2;
        static constexpr int cv = 2;
        static constexpr cppad::backward::var a = av;
        static constexpr cppad::backward::var b = bv;
        static constexpr cppad::backward::let c = cv;
        constexpr auto expression = a*a*b*c;
        static_assert(derivative_of(expression, wrt(a), cppad::first_order) == 2*av*bv*cv);
        static_assert(derivative_of(expression, wrt(a), cppad::second_order) == 2*bv*cv);
        static_assert(derivative_of(expression, wrt(a), cppad::third_order) == 0);
    };

    "bw_variables_of_test"_test = [] () {
        constexpr cppad::backward::var a = 1;
        constexpr cppad::backward::var b = 3;
        constexpr cppad::backward::var c = 5;
        const auto expr = c + (a + b)*b;
        const auto vars = cppad::backward::variables_of(expr);
        const auto& ref_a = std::get<const std::remove_cvref_t<decltype(a)>&>(vars);
        const auto& ref_b = std::get<const std::remove_cvref_t<decltype(b)>&>(vars);
        const auto& ref_c = std::get<const std::remove_cvref_t<decltype(c)>&>(vars);
        expect(&a == &ref_a);
        expect(&b == &ref_b);
        expect(&c == &ref_c);
        const auto derivs = derivatives_of(expr, vars);
        expect(eq(derivs[a], 3));
        expect(eq(derivs[b], 4 + 3));
        expect(eq(derivs[c], 1));
    };

    "bw_gradient_test"_test = [] () {
        static constexpr cppad::backward::var a = 1;
        static constexpr cppad::backward::var b = 3;
        static constexpr cppad::backward::var c = 5;
        constexpr auto expr = c + (a + b)*b;
        {
            const auto grad = gradient_of(expr);
            expect(eq(grad[a], 3));
            expect(eq(grad[b], 4 + 3));
            expect(eq(grad[c], 1));
        }
        {
            constexpr auto grad = gradient_of(expr);
            static_assert(grad[a] == 3);
            static_assert(grad[b] == 4 + 3);
            static_assert(grad[c] == 1);
        }
    };

    return EXIT_SUCCESS;
}