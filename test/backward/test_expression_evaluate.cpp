#include <cstdlib>

#include <boost/ut.hpp>

#include <cppad/backward/symbols.hpp>
#include <cppad/backward/evaluate.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using cppad::backward::var;
using cppad::backward::let;
using cppad::backward::expression;


int main(int argc, char** argv) {

    "var_bindings"_test = [] () {
        constexpr var a;
        constexpr var b;
        constexpr auto bindings = at(a = 42, b = 43);
        static_assert(bindings[a] == 42);
        static_assert(bindings[b] == 43);
    };

    "var_reference_bindings"_test = [] () {
        var a;
        var b;
        auto binding_a = a = 42;
        auto binding_b = b = 43;
        auto bindings = at(binding_a, binding_b);
        expect(eq(bindings[a], 42));
        expect(eq(bindings[b], 43));
    };

    "var_evaluate"_test = [] () {
        constexpr var a;
        constexpr auto result = evaluate(a, at(a = 2.0));
        static_assert(result == 2.0);
    };

    "let_evaluate"_test = [] () {
        constexpr let a;
        constexpr auto result = evaluate(a, at(a = 2.0));
        static_assert(result == 2.0);
    };

    "exp_expression_evaluate"_test = [] () {
        var a;
        let b;
        auto exp_a = a.exp();
        auto exp_b = std::exp(b);
        expect(eq(evaluate(exp_a, at(a = 2.0)), std::exp(2.0)));
        expect(eq(evaluate(exp_b, at(b = 4.0)), std::exp(4.0)));
    };

    "composite_expression_evaluate"_test = [] () {
        var a;
        let b;
        expression result = std::exp((a + b)*a);
        expect(eq(
            evaluate(result, at(a = 2.0, b = 4.0)),
            std::exp((2.0 + 4.0)*2.0)
        ));
    };

    "composite_expression_evaluate_via_operator()"_test = [] () {
        var a;
        let b;
        expression result = std::exp((a + b)*a);
        expect(eq(
            result(a = 2.0, b = 4.0),
            std::exp((2.0 + 4.0)*2.0)
        ));
    };

    "composite_expression_evaluate_additional_vars"_test = [] () {
        var a;
        let b;
        var c;
        expression result = std::exp((a + b)*a);
        expect(eq(
            evaluate(result, at(a = 2.0, b = 4.0, c = 10.0)),
            std::exp((2.0 + 4.0)*2.0)
        ));
    };

    "compile_time_expression_evaluate"_test = [] () {
        static constexpr var a;
        static constexpr let b;
        constexpr auto formula = (a + b)*a;
        static_assert(evaluate(formula, at(a = 2.0, b = 4.0)) == 12.0);
    };

    return EXIT_SUCCESS;
}
