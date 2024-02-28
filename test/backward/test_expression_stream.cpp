#include <cstdlib>
#include <sstream>

#include <boost/ut.hpp>

#include <cppad/backward/symbols.hpp>
#include <cppad/backward/differentiate.hpp>
#include <cppad/backward/stream.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using cppad::backward::let;

int main(int argc, char** argv) {

    "expression_simple_stream"_test = [] () {
        let a;
        let b;
        std::ostringstream s;
        stream(s, a + b, with(a = "a", b = "b"));
        expect(eq(std::string{s.str()}, std::string{"a + b"}));
    };

    "expression_nested_stream"_test = [] () {
        let a;
        let b;
        let c;
        auto expr = std::exp(a + b)*c + b*(a + b);
        std::ostringstream s;
        stream(s, expr, with(a = "a", b = "b", c = "c"));
        expect(eq(std::string{s.str()}, std::string{"(exp(a + b))*(c) + (b)*(a + b)"}));
    };

    // TODO: fix expression simplification issue
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
