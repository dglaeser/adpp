#include <cstdlib>
#include <sstream>
#include <format>

#include <boost/ut.hpp>

#include <adpp/backward/symbols.hpp>
#include <adpp/backward/operators.hpp>
#include <adpp/backward/differentiate.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using adpp::backward::var;
using adpp::backward::let;
using adpp::backward::cval;

int main() {

    "expression_simple_stream"_test = [] () {
        let a;
        let b;
        std::ostringstream s;
        s << (a + b).with(a = "a", b = "b");
        expect(eq(std::string{s.str()}, std::string{"a + b"}));
    };

    "negated_expression_simple_stream"_test = [] () {
        let a;
        std::ostringstream s;
        auto expr = -a;
        s << expr.with(a = "a");
        expect(eq(std::string{s.str()}, std::string{"-1*a"}));
    };

    "expression_nested_stream"_test = [] () {
        let a;
        let b;
        let c;
        auto expr = exp(a + b)*c + b*(a + b);
        auto text = std::format("{}", expr.with(a = "a", b = "b", c = "c"));
        expect(eq(text, std::string{"(exp(a + b))*c + b*(a + b)"}));
    };

    "expression_no_braces_stream"_test = [] () {
        let a;
        let b;
        let c;
        auto expr = a + b + c;
        expect(eq(
            std::format("{}", expr.with(a = "a", b = "b", c = "c")),
            std::string{"a + b + c"}
        ));
    };

    "expression_complex_stream"_test = [] () {
        var x;
        var y;
        let mu;
        std::stringstream s;
        s << sqrt(log(exp((x + y/x)*mu))).with(x = "x", y = "y", mu = "µ");
        expect(eq(s.str(), std::string{"√(log(exp((x + y/x)*µ)))"}));
    };

    "expression_multiplication_derivative_simplification"_test  = [] () {
        var x;
        {
            const auto expr = cval<2>*x;
            const auto deriv = expr.differentiate(wrt(x));
            expect(eq(deriv.evaluate(at(x = 2)), 2));
            std::stringstream s;
            s << deriv.with(x = "x");
            expect(eq(s.str(), std::string{"2"}));
        }
        {
            var y;
            const auto expr = x*y + y;
            const auto deriv_x = expr.differentiate(wrt(x)); {
                expect(eq(deriv_x.evaluate(at(x = 2, y = 3)), 3));
                std::stringstream s;
                s << deriv_x.with(x = "x", y = "y");
                expect(eq(s.str(), std::string{"1*y"}));
            }
            const auto deriv_y = expr.differentiate(wrt(y)); {
                expect(eq(deriv_y.evaluate(at(x = 2, y = 3)), 2 + 1));
                std::stringstream s;
                s << deriv_y.with(x = "x", y = "y");
                expect(eq(s.str(), std::string{"x*1 + 1"}));
            }
        }
    };

    "expression_division_derivative_simplification"_test  = [] () {
        var x;
        var y;
        const auto expr = x/y + (y + y/cval<2.0>);
        const auto deriv_x = expr.differentiate(wrt(x)); {
            expect(eq(deriv_x.evaluate(at(x = 2.0, y = 3.0)), 1.0/3.0));
        }
        const auto deriv_y = expr.differentiate(wrt(y)); {
            expect(eq(deriv_y.evaluate(at(x = 2.0, y = 3.0)), ((-1*2.0)*1)/(3.0*3.0) + 1.5));
            std::stringstream s;
            s << deriv_y.with(x = "x", y = "y");
            expect(eq(s.str(), std::string{"((-1*x)*1)/(y*y) + 1.5"}));
        }
    };

    "bound_expression_derivative_io"_test = [] () {
        static constexpr var a;
        static constexpr let b;
        static constexpr auto derivative = ((a + b)*a).with(a = 2.0, b = 4.0).differentiate(wrt(a));
        std::stringstream s;
        derivative.export_to(s);
        expect(eq(s.str(), std::string{"1*2 + (2 + 4)*1"}));
    };

    return EXIT_SUCCESS;
}
