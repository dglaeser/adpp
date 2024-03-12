#include <cstdlib>
#include <sstream>

#include <boost/ut.hpp>

#include <adpp/backward/symbols.hpp>
#include <adpp/backward/operators.hpp>
#include <adpp/backward/differentiate.hpp>
#include <adpp/backward/io.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using adpp::backward::let;
using adpp::backward::formatted;

int main() {

    "expression_simple_stream"_test = [] () {
        let a;
        let b;
        std::ostringstream s;
        s << formatted{a + b, where(a = "a", b = "b")};
        expect(eq(std::string{s.str()}, std::string{"a + b"}));
    };

    "expression_nested_stream"_test = [] () {
        let a;
        let b;
        let c;
        auto expr = exp(a + b)*c + b*(a + b);
        auto text = formatted{expr, where(a = "a", b = "b", c = "c")}.to_string();
        expect(eq(text, std::string{"(exp(a + b))*c + b*(a + b)"}));
    };

    "expression_no_braces_stream"_test = [] () {
        let a;
        let b;
        let c;
        auto expr = a + b + c;
        expect(eq(
            formatted{expr, where(a = "a", b = "b", c = "c")}.to_string(),
            std::string{"a + b + c"}
        ));
    };

    // TODO(?): this would require simplifications of expressions
    // "expression_derivative_stream"_test = [] () {
    //     let a;
    //     let b;
    //     let c;
    //     auto expr = a*b*c;
    //     auto deriv = differentiate(expr, wrt(a));
    //     std::ostringstream s;
    //     stream(s, deriv, with(a = "a", b = "b", c = "c"));
    //     expect(eq(std::string{s.str()}, std::string{"b*c"}));
    // };

    return EXIT_SUCCESS;
}
