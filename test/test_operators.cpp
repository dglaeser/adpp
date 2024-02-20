#include <cstdlib>
#include <cstddef>

#include <boost/ut.hpp>

#include <cppad/constant.hpp>
#include <cppad/variable.hpp>
#include <cppad/operators.hpp>

int main() {
    using boost::ut::operator""_test;
    using boost::ut::expect;
    using boost::ut::eq;

    "plus_operator_one_level"_test = [] () {
        const auto a = cppad::var(2.0);
        const auto b = cppad::var(4.0);
        const auto c = a + b;
        expect(eq(c.value(), 6.0));
        expect(eq(c.partial(a), 1.0));
        expect(eq(c.partial(b), 1.0));
    };

    "plus_times_operators_one_level_with_constants"_test = [] () {
        const auto a = cppad::var(2);
        const auto b = cppad::var(4);
        const auto c = a*cppad::constant(42) + b*cppad::constant(48);
        expect(eq(c.value(), 42*2 + 48*4));
        expect(eq(c.partial(a), 42));
        expect(eq(c.partial(b), 48));
    };

    "plus_times_operators_three_levels_with_constants"_test = [] () {
        const auto a = cppad::var(2);
        const auto b = cppad::var(4);
        const auto c = a*cppad::constant(42) + b*cppad::constant(48);
        const auto d = c*a;
        expect(eq(d.value(), (42*2 + 48*4)*2));
        expect(eq(c.partial(a), 42));
        expect(eq(d.partial(a), (42*2 + 48*4) + 42*2));
    };

    "plus_times_operators_three_levels_with_arithmetics"_test = [] () {
        const auto a = cppad::var(2);
        const auto b = cppad::var(4);
        const auto c = a*42 + b*48;
        const auto d = c*a;
        expect(eq(d.value(), (42*2 + 48*4)*2));
        expect(eq(c.partial(a), 42));
        expect(eq(d.partial(a), (42*2 + 48*4) + 42*2));
    };

    "plus_times_operators_three_levels_single_expression"_test = [] () {
        const auto a = cppad::var(2);
        const auto d = (a*cppad::constant(42) + cppad::var(4)*cppad::constant(48))*a;
        expect(eq(d.value(), (42*2 + 48*4)*2));
        expect(eq(d.partial(a), (42*2 + 48*4) + 42*2));
    };

    "plus_times_expression_modified_variable"_test = [] () {
        auto a = cppad::var(42);
        auto b = cppad::var(2);
        auto c = a*b;
        a *= 2;
        expect(eq(c.value(), 168));
        expect(eq(c.partial(a), 2));

        a /= 2;
        expect(eq(c.value(), 84));
        expect(eq(c.partial(a), 2));

        a.set(5);
        expect(eq(c.value(), 10));
        expect(eq(c.partial(a), 2));
    };

    "exp_expression"_test = [] () {
        auto a = cppad::var(3);
        auto e = (a*2).exp();
        expect(eq(e.value(), std::exp(3*2)));
        expect(eq(e.partial(a), std::exp(3*2)*2));
    };

    "exp_expression_with_std_function"_test = [] () {
        auto a = cppad::var(3);
        auto e = std::exp(a*2);
        expect(eq(e.value(), std::exp(3*2)));
        expect(eq(e.partial(a), std::exp(3*2)*2));
    };

    "plus_times_expression_at_compile_time"_test = [] () {
        static constexpr auto a = cppad::var(1);
        static constexpr auto b = cppad::var(2);
        constexpr auto c = (a + b)*b;
        static_assert(c.value() == 6);
        static_assert(c.partial(a) == 2);
        static_assert(c.partial(b) == 5);
    };

    return EXIT_SUCCESS;
}
