#include <cstdlib>

#include <iostream>
#include <boost/ut.hpp>

#include <cppad/backward/var.hpp>
#include <cppad/backward/let.hpp>
#include <cppad/backward/format.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using cppad::backward::var;
using cppad::backward::let;
using cppad::backward::expression;

void test_expression_equality(auto&& expression_factory) {
    auto first = expression_factory();
    auto second = expression_factory();
    static_assert(std::is_same_v<
        std::remove_cvref_t<decltype(first)>,
        std::remove_cvref_t<decltype(second)>
    >);
}

int main() {
    using boost::ut::operator""_test;
    using boost::ut::expect;
    using boost::ut::eq;

    "plus_operator_one_level"_test = [] () {
        const var a{2.0};
        const var b{4.0};
        const auto c = a + b;
        expect(eq(c.value(), 6.0));
        expect(eq(derivative_of(c, wrt(a)), 1.0));
        expect(eq(derivative_of(c, wrt(b)), 1.0));
    };

    "plus_times_operators_one_level_with_constants"_test = [] () {
        const var a{2};
        const var b{4};
        const auto c = a*let{42} + b*let{48};
        expect(eq(c.value(), 42*2 + 48*4));
        expect(eq(derivative_of(c, wrt(a)), 42));
        expect(eq(derivative_of(c, wrt(b)), 48));
    };

    "plus_times_operators_three_levels_with_constants"_test = [] () {
        const var a{2};
        const var b{4};
        const auto c = a*let{42} + b*let{48};
        const auto d = c*a;
        expect(eq(d.value(), (42*2 + 48*4)*2));
        expect(eq(derivative_of(c, wrt(a)), 42));
        expect(eq(derivative_of(d, wrt(a)), (42*2 + 48*4) + 42*2));
    };

    "plus_times_operators_three_levels_with_arithmetics"_test = [] () {
        const var a{2};
        const var b{4};
        const auto c = a*42 + b*48;
        const auto d = c*a;
        expect(eq(d.value(), (42*2 + 48*4)*2));
        expect(eq(derivative_of(c, wrt(a)), 42));
        expect(eq(derivative_of(d, wrt(a)), (42*2 + 48*4) + 42*2));
    };

    "plus_times_operators_three_levels_single_expression"_test = [] () {
        const var a{2};
        const auto d = (a*let{42} + var{4}*let{48})*a;
        expect(eq(d.value(), (42*2 + 48*4)*2));
        expect(eq(derivative_of(d, wrt(a)), (42*2 + 48*4) + 42*2));
    };

    "plus_times_expression_modified_variable"_test = [] () {
        var a{42};
        var b{2};
        auto c = a*b;
        a *= 2;
        expect(eq(c.value(), 168));
        expect(eq(derivative_of(c, wrt(a)), 2));

        a /= 2;
        expect(eq(c.value(), 84));
        expect(eq(derivative_of(c, wrt(a)), 2));

        a = 5;
        expect(eq(c.value(), 10));
        expect(eq(derivative_of(c, wrt(a)), 2));
    };

    "exp_expression"_test = [] () {
        var a{3};
        auto e = (a*2).exp();
        expect(eq(e.value(), std::exp(3*2)));
        expect(eq(derivative_of(e, wrt(a)), std::exp(3*2)*2));
    };

    "exp_expression_with_std_function"_test = [] () {
        var a{3};
        auto e = std::exp(a*2);
        expect(eq(e.value(), std::exp(3*2)));
        expect(eq(derivative_of(e, wrt(a)), std::exp(3*2)*2));
    };

    "plus_times_expression_at_compile_time"_test = [] () {
        static constexpr var a{1};
        static constexpr var b{2};
        constexpr auto c = (a + b)*b;
        static_assert(c.value() == 6);
        static_assert(derivative_of(c, wrt(a)) == 2);
        static_assert(derivative_of(c, wrt(b)) == 5);
    };

    "expressions_type_equality"_test = [] () {
        auto a = var{3};
        test_expression_equality([&] () { return a + a; });
        test_expression_equality([&] () { return a*a; });
        test_expression_equality([&] () { return a.exp(); });
    };

    "plus_expr_derivative_expression"_test = [] () {
        static constexpr var a = 1;
        static constexpr var b = 3;
        static constexpr expression e = a + b*2;
        constexpr expression de_db = differentiate(e, wrt(b));
        static_assert(de_db.value() == 2);
        expect(eq(de_db.value(), 2));
    };

    "minus_expr_derivative_expression"_test = [] () {
        static constexpr var a = 1;
        static constexpr var b = 3;
        static constexpr expression e = a - b*2;
        constexpr expression de_db = differentiate(e, wrt(b));
        static_assert(de_db.value() == -2);
        expect(eq(de_db.value(), -2));
    };

    "times_expr_derivative_expression"_test = [] () {
        static constexpr var a = 1;
        static constexpr var b = 3;
        static constexpr expression e = a*b*2;
        constexpr expression de_da = differentiate(e, wrt(a));
        constexpr expression de_db = differentiate(e, wrt(b));
        static_assert(de_da.value() == 2*3);
        static_assert(de_db.value() == 2*1);
        expect(eq(de_da.value(), 2*3));
        expect(eq(de_db.value(), 2*1));
    };

    "complex_expr_derivative_expression"_test = [] () {
        var a = 1;
        var b = 3;
        expression e = std::exp(a + b)*(b - 1);
        expression de_da = differentiate(e, wrt(a));
        expression de_db = differentiate(e, wrt(b));
        expect(eq(de_da.value(), std::exp(1 + 3)*(3 - 1)));
        expect(eq(de_db.value(), std::exp(1 + 3)*3));
    };

    "name_map"_test = [] () {
        var a = 1;
        var b = 2;
        auto map = with_names(a |= "a", b |= "b");
        static_assert(map.contains(a));
        static_assert(map.contains(b));
        expect(eq(std::string{map.name_of(a)}, std::string{"a"}));
        expect(eq(std::string{map.name_of(b)}, std::string{"b"}));
    };

    "expression_stream"_test = [] () {
        std::ostringstream s;
        auto a = var{1};
        auto b = var{2};
        auto c = var{3};
        auto expr = ((a + b)*c).exp();
        cppad::backward::stream(s, expr, with_names(
            a |= "a",
            b |= "b",
            c |= "c"
        ));
        expect(eq(s.str(), std::string{"exp((a + b)*c)"}));
    };

    "expression_with_constant_stream"_test = [] () {
        std::ostringstream s;
        var<double> x;
        let<double> mu;
        auto expr = (mu*x).exp();
        cppad::backward::stream(s, expr, with_names(
            x |= "x",
            mu |= "µ"
        ));
        expect(eq(s.str(), std::string{"exp(µ*x)"}));
    };

    "expression_derivative_stream"_test = [] () {
        std::ostringstream s;
        auto a = var{1};
        auto b = var{2};
        auto c = var{3};
        auto expr = ((a + b)*c).exp();
        auto deriv = differentiate(expr, wrt(a));
        cppad::backward::stream(s, deriv, with_names(
            a |= "a",
            b |= "b",
            c |= "c"
        ));
        expect(eq(s.str(), std::string{"(exp((a + b)*c))*((1 + 0)*c + (a + b)*0)"}));
    };

    return EXIT_SUCCESS;
}

