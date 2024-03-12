#include <cstdlib>
#include <functional>
#include <type_traits>
#include <iostream>
#include <sstream>

#include <boost/ut.hpp>

#include <adpp/backward/symbols.hpp>
#include <adpp/backward/expression.hpp>
#include <adpp/backward/differentiate.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using adpp::backward::let;
using adpp::backward::var;
using adpp::backward::cval;
using adpp::backward::expression;

int main() {

    "expression_type"_test = [] () {
        var x;
        var y;

        using X = std::remove_cvref_t<decltype(x)>;
        using Y = std::remove_cvref_t<decltype(y)>;
        using E1 = expression<std::multiplies<void>, X, Y>;
        using E2 = expression<std::plus<void>, E1, X>;

        expect(eq(E2{}.evaluate(at(x = 3, y = 2)), 9));
    };

    "expression_operators_single_expression"_test = [] () {
        var x;
        var y;
        const auto expr = exp((x + y)*(x - y)/(x + y));
        expect(eq(expr.evaluate(at(x = 3, y = 2)), std::exp(1.0)));
    };

    "expression_operators_multiple_expression"_test = [] () {
        var x;
        var y;
        const auto expr_tmp_1 = (x + y)*(x - y)/(x + y);
        const auto expr_tmp_2 = exp(expr_tmp_1);
        const auto expr = cval<2>*expr_tmp_2;
        expect(eq(expr.evaluate(at(x = 3, y = 2)), 2*std::exp(1.0)));
    };

    "expression_derivative"_test = [] () {
        var x;
        var y;
        var z;
        const auto expr = x - 2*y - x + exp(cval<3.0>/z);
        const auto derivs = derivatives_of(expr, wrt(x, y, z), at(x = 3, y = 2, z = 4.0));
        expect(eq(derivs[x], 0));
        expect(eq(derivs[y], -2));
        expect(eq(derivs[z], std::exp(3.0/4.0)*(-3.0/16.0)));
    };

    "expression_gradient"_test = [] () {
        var x;
        var y;
        const auto expression = cval<2>*(x + y)*x;
        const auto gradient = grad(expression, at(x = 3, y = 2));
        expect(eq(gradient[x], 4.0*3.0 + 2.0*2.0));
        expect(eq(gradient[y], 2.0*3.0));
    };

    "expression_derivative_wrt_expression"_test = [] () {
        var x;
        var y;
        const auto tmp = x + y;
        const auto expr = cval<2>*tmp;
        const auto [value, derivs] = expr.template back_propagate<double>(at(x = 3, y = 2), wrt(tmp));
        expect(eq(value, 10));
        expect(eq(derivs[tmp], 2));
    };

     "expression_stream"_test = [] () {
        var x;
        var y;
        let mu;
        std::stringstream s;
        exp((x + y/x)*mu).export_to(s, with(x = "x", y = "y", mu = "µ"));
        expect(eq(s.str(), std::string{"exp((x + y/x)*µ)"}));
    };

    "expression_derivative"_test  = [] () {
        var x;
        var y;
        let mu;
        const auto expr = cval<1.5>*(x + y) - exp(cval<-1>*mu*x);
        const auto deriv = expr.differentiate_wrt(wrt(x));
        expect(eq(deriv.evaluate(at(x = 2, y = 3, mu = 4)), 1.5 - std::exp(-1.0*4*2)*(-1.0*4)));
    };

    "expression_multiplication_derivative_simplification"_test  = [] () {
        var x;
        {
            const auto expr = cval<2>*x;
            const auto deriv = expr.differentiate_wrt(wrt(x));
            expect(eq(deriv.evaluate(at(x = 2)), 2));
            std::stringstream s;
            print_to(s, deriv, with(x = "x"));
            expect(eq(s.str(), std::string{"2"}));
        }
        {
            var y;
            const auto expr = x*y + y;
            const auto deriv_x = expr.differentiate_wrt(wrt(x)); {
                expect(eq(deriv_x.evaluate(at(x = 2, y = 3)), 3));
                std::stringstream s;
                print_to(s, deriv_x, with(x = "x", y = "y"));
                expect(eq(s.str(), std::string{"1*y"}));
            }
            const auto deriv_y = expr.differentiate_wrt(wrt(y)); {
                expect(eq(deriv_y.evaluate(at(x = 2, y = 3)), 2 + 1));
                std::stringstream s;
                print_to(s, deriv_y, with(x = "x", y = "y"));
                expect(eq(s.str(), std::string{"x*1 + 1"}));
            }
        }
    };

    "expression_division_derivative_simplification"_test  = [] () {
        var x;
        var y;
        const auto expr = x/y + (y + y/cval<2.0>);
        const auto deriv_x = expr.differentiate_wrt(wrt(x)); {
            expect(eq(deriv_x.evaluate(at(x = 2.0, y = 3.0)), 1.0/3.0));
        }
        const auto deriv_y = expr.differentiate_wrt(wrt(y)); {
            expect(eq(deriv_y.evaluate(at(x = 2.0, y = 3.0)), ((-1*2.0)*1)/(3.0*3.0) + 1.5));
            std::stringstream s;
            print_to(s, deriv_y, with(x = "x", y = "y"));
            expect(eq(s.str(), std::string{"((-1*x)*1)/(y*y) + 1.5"}));
        }
    };

    return EXIT_SUCCESS;
}
