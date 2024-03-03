#include <cstdlib>
#include <array>

#include <boost/ut.hpp>

#include <adpp/common.hpp>
#include <adpp/backward/symbols.hpp>
#include <adpp/backward/evaluate.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using adpp::backward::var;
using adpp::backward::let;
using adpp::backward::function;


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
        function f = std::exp((a + b)*a);
        expect(eq(
            evaluate(f, at(a = 2.0, b = 4.0)),
            std::exp((2.0 + 4.0)*2.0)
        ));
    };

    "composite_expression_evaluate_via_operator()"_test = [] () {
        var a;
        let b;
        function result = std::exp((a + b)*a);
        expect(eq(
            result(a = 2.0, b = 4.0),
            std::exp((2.0 + 4.0)*2.0)
        ));
    };

    "composite_expression_evaluate_additional_vars"_test = [] () {
        var a;
        let b;
        var c;
        function result = std::exp((a + b)*a);
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

    "expression_from_index_reduce"_test = [] () {
        static constexpr std::array vec{0, 1, 2, 3, 4};
        constexpr function f = adpp::recursive_reduce(
            adpp::index_range<0, vec.size()>{},
            [&] <auto i> (adpp::index_constant<i>, auto&& current) {
                return std::move(current) + vec[i];
            },
            adpp::backward::val{0}
        );
        static_assert(f() == 10);
    };

    "expression_from_index_reduce_runtime"_test = [] () {
        std::array vec{0, 1, 2, 3, 4};
        function f = adpp::recursive_reduce(
            adpp::index_range<0, vec.size()>{},
            [&] <auto i> (adpp::index_constant<i>, auto&& current) {
                return std::move(current) + vec[i];
            },
            adpp::backward::val{0}
        );
        expect(eq(f(), 10));
    };

    "expression_template_from_index_reduce"_test = [] () {
        static constexpr std::array vec{0, 1, 2, 3, 4};
        static constexpr let initial;
        constexpr function f = adpp::recursive_reduce(
            adpp::index_range<0, vec.size()>{},
            [&] <auto i, typename T> (adpp::index_constant<i>, T&& current) {
                return std::forward<T>(current) + vec[i];
            },
            initial
        );
        static_assert(f(initial = 0) == 10);
    };

    "expression_template_from_index_reduce_runtime"_test = [] () {
        std::array vec{0, 1, 2, 3, 4};
        let initial;
        function f = adpp::recursive_reduce(
            adpp::index_range<0, vec.size()>{},
            [&] <auto i, typename T> (adpp::index_constant<i>, T&& current) {
                return std::forward<T>(current) + vec[i];
            },
            initial
        );
        expect(eq(f(initial = 0), 10));
    };

    // TODO: vector scalar product?

    return EXIT_SUCCESS;
}
