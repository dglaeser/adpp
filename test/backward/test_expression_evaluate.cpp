#include <cstdlib>

#include <boost/ut.hpp>

#include <adpp/backward/symbols.hpp>
#include <adpp/backward/operators.hpp>
#include <adpp/backward/evaluate.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using adpp::backward::var;
using adpp::backward::let;
using adpp::backward::cval;
using adpp::backward::function;
using adpp::backward::expression;

int main() {

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

    "exp_expression_evaluate"_test = [] () {
        var a;
        let b;
        auto exp_a = exp(a);
        auto exp_b = exp(b);
        expect(eq(evaluate(exp_a, at(a = 2.0)), std::exp(2.0)));
        expect(eq(evaluate(exp_b, at(b = 4.0)), std::exp(4.0)));
    };

    "function_evaluate"_test = [] () {
        var a;
        let b;
        function f = exp((a + b)*a);
        expect(eq(
            evaluate(f, at(a = 2.0, b = 4.0)),
            std::exp((2.0 + 4.0)*2.0)
        ));
    };

    "function_evaluate_via_operator()"_test = [] () {
        var a;
        let b;
        function result = exp((a + b)*a);
        expect(eq(
            result(a = 2.0, b = 4.0),
            std::exp((2.0 + 4.0)*2.0)
        ));
    };

    "function_evaluate_additional_vars"_test = [] () {
        var a;
        let b;
        var c;
        function result = exp((a + b)*a);
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
