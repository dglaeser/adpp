#include <cstdlib>
#include <functional>
#include <type_traits>

#include <boost/ut.hpp>

#include <adpp/backward/symbols.hpp>
#include <adpp/backward/expression.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using adpp::backward::var;
using adpp::backward::aval;
using adpp::backward::expression;

int main() {

    "expression_type"_test = [] () {
        var x;
        var y;

        using X = std::remove_cvref_t<decltype(x)>;
        using Y = std::remove_cvref_t<decltype(y)>;
        using E1 = expression<std::multiplies<void>, X, Y>;
        using E2 = expression<std::plus<void>, E1, X>;

        expect(eq(E2{}(at(x = 3, y = 2)), 9));
    };

    "expression_operators_single_expression"_test = [] () {
        var x;
        var y;
        const auto expr = exp((x + y)*(x - y)/(x + y));
        expect(eq(expr(at(x = 3, y = 2)), std::exp(1.0)));
    };

    "expression_operators_multiple_expression"_test = [] () {
        var x;
        var y;
        const auto expr_tmp_1 = (x + y)*(x - y)/(x + y);
        const auto expr_tmp_2 = exp(expr_tmp_1);
        const auto expr = aval<2>*expr_tmp_2;
        expect(eq(expr(at(x = 3, y = 2)), 2*std::exp(1.0)));
    };

    // TODO: Test division
    "expression_derivative"_test = [] () {
        var x;
        var y;
        var z;
        const auto expr = x - aval<2>*y - x + exp(aval<3.0>*z);
        const auto derivs = expr.back_propagate(at(x = 3, y = 2, z = 4.0), wrt(x, y, z));
        expect(eq(derivs.second[x], 0));
        expect(eq(derivs.second[y], -2));
        expect(eq(derivs.second[z], std::exp(12.0)*3.0));
    };

    "expression_gradient"_test = [] () {
        var x;
        var y;
        const auto expression = aval<2>*(x + y)*x;
        const auto grad = expression.gradient(at(x = 3, y = 2));
        expect(eq(grad[x], 4.0*3.0 + 2.0*2.0));
        expect(eq(grad[y], 2.0*3.0));
    };

    "expression_derivative_wrt_expression"_test = [] () {
        var x;
        var y;
        const auto tmp = x + y;
        const auto expr = aval<2>*tmp;
        const auto derivs = expr.back_propagate(at(x = 3, y = 2), wrt(tmp));
        expect(eq(derivs.second[tmp], 2));
    };

    return EXIT_SUCCESS;
}
