#include <cstdlib>

#include <boost/ut.hpp>

#include <adpp/backward/symbols.hpp>
#include <adpp/backward/operators.hpp>
#include <adpp/backward/evaluate.hpp>
#include <adpp/backward/differentiate.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using adpp::backward::var;
using adpp::backward::let;
using adpp::backward::cval;
using adpp::backward::function;

template<typename T> struct expression_type;
template<typename T> struct bindings_type;

template<typename E, typename B>
struct expression_type<adpp::backward::bound_expression<E, B>> : std::type_identity<E> {};
template<typename E, typename B>
struct bindings_type<adpp::backward::bound_expression<E, B>> : std::type_identity<B> {};

int main() {

    "derivatives"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        constexpr auto expr = (a + b)*b;
        constexpr auto derivs = derivatives_of(expr, wrt(a, b), at(a = 1.0, b = 2.0));
        static_assert(derivs[a] == 2.0);
        static_assert(derivs[b] == 2.0 + 3.0);
    };

    "derivatives_at_runtime"_test = [] () {
        var a;
        var b;
        auto expr = exp((a + b)*b);
        auto derivs = derivatives_of(expr, wrt(a, b), at(a = 1.0, b = 2.0));
        expect(eq(derivs[a], std::exp((1.0 + 2.0)*2.0)*2.0));
        expect(eq(derivs[b], std::exp((1.0 + 2.0)*2.0)*(2.0 + 3.0)));
    };

    "derivative_wrt_single_var"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        constexpr auto expr = (a + b)*b;
        static_assert(2.0 == derivative_of(expr, wrt(a), at(a = 1.0, b = 2.0)));
    };

    "negated_expression_derivative"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        constexpr auto expr = -b*(a + b);
        static_assert(-5.0 == derivative_of(expr, wrt(b), at(a = 1.0, b = 2.0)));
    };

    "expression_gradient"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        static constexpr let mu;
        constexpr auto expr = (a + b)*b*mu;
        constexpr auto gradient = grad(expr, at(a = 1.0, b = 2.0, mu = 3.0));
        static_assert(2.0*3.0 == gradient[a]);
        static_assert(5.0*3.0 == gradient[b]);
    };

    "expression_with_cval_derivative"_test = [] () {
        using namespace adpp::indices;
        var x;
        var y;
        var z;
        const auto expr = x - 2*y - x + exp(cval<3.0>/z);
        const auto derivs = derivatives_of(expr, wrt(x, y, z), at(x = 3, y = 2, z = 4.0));
        expect(eq(derivs[x], 0));
        expect(eq(derivs[_0], 0));

        expect(eq(derivs[y], -2));
        expect(eq(derivs[_1], -2));

        expect(eq(derivs[z], std::exp(3.0/4.0)*(-3.0/16.0)));
        expect(eq(derivs[_2], std::exp(3.0/4.0)*(-3.0/16.0)));
    };

    "expression_with_cval_gradient"_test = [] () {
        var x;
        var y;
        const auto expression = cval<2>*(x + y)*x;
        const auto gradient = grad(expression, at(x = 3, y = 2));
        expect(eq(gradient[x], 4.0*3.0 + 2.0*2.0));
        expect(eq(gradient[y], 2.0*3.0));
    };

    "expression_with_val_derivative"_test  = [] () {
        var x;
        var y;
        let mu;
        const auto expr = 1.5*(x + y) - exp(-1*mu*x);
        const auto deriv = expr.differentiate(wrt(x));
        expect(eq(deriv.evaluate(at(x = 2, y = 3, mu = 4)), 1.5 - std::exp(-1.0*4*2)*(-1.0*4)));
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

    "higher_order_derivatives"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        static constexpr let mu;
        constexpr auto expr = a*a*a + b*b*cval<2> + mu;
        static_assert(3.0 == derivative_of(expr, wrt(a), at(a = 1.0, b = 2.0, mu = 3.0), adpp::first_order));
        static_assert(6.0 == derivative_of(expr, wrt(a), at(a = 1.0, b = 2.0, mu = 3.0), adpp::second_order));
        static_assert(6.0 == derivative_of(expr, wrt(a), at(a = 1.0, b = 2.0, mu = 3.0), adpp::third_order));

        static_assert(8.0 == derivative_of(expr, wrt(b), at(a = 1.0, b = 2.0, mu = 3.0), adpp::first_order));
        static_assert(4.0 == derivative_of(expr, wrt(b), at(a = 1.0, b = 2.0, mu = 3.0), adpp::second_order));
        static_assert(0.0 == derivative_of(expr, wrt(b), at(a = 1.0, b = 2.0, mu = 3.0), adpp::third_order));
    };

    "derivative_mixed_var_let_expression"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        static constexpr let mu;
        static constexpr auto expr = (a + b)*b*mu;
        {
            constexpr auto derivative = differentiate(expr, wrt(a));
            static_assert(evaluate(derivative, at(a = 1.0, b = 2.0, mu = 3.0)) == 6.0);
        }
        {
            constexpr auto derivative = differentiate(expr, wrt(b));
            static_assert(evaluate(derivative, at(a = 1.0, b = 2.0, mu = 3.0)) == 6.0 + 9.0);
        }
        {
            constexpr auto derivative = differentiate(expr, wrt(mu));
            static_assert(evaluate(derivative, at(a = 1.0, b = 2.0, mu = 3.0)) == 6.0);
        }
    };

    "derivative_expression_disappearing_variables"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        static constexpr var c;
        static constexpr var d;
        static constexpr let mu;
        static constexpr auto expr = cval<2>*a + b*mu - c + cval<1>/d;
        {
            constexpr function f = differentiate(expr, wrt(a));
            static_assert(f() == 2.0);
            expect(eq(f(), 2.0));
        }
        {
            constexpr auto derivative = differentiate(expr, wrt(b));
            static_assert(evaluate(derivative, at(mu = 3.0)) == 3.0);
            expect(eq(evaluate(derivative, at(mu = 3.0)), 3.0));
        }
        {
            constexpr auto derivative = differentiate(expr, wrt(mu));
            static_assert(evaluate(derivative, at(b = 2.0)) == 2.0);
            expect(eq(evaluate(derivative, at(b = 2.0)), 2.0));
        }
        {
            constexpr function f = differentiate(expr, wrt(c));
            static_assert(evaluate(f, adpp::backward::at()) == -1.0);
            static_assert(f() == -1.0);
            expect(eq(evaluate(f, adpp::backward::at()), -1.0));
            expect(eq(f(), -1.0));
        }
        {
            constexpr auto derivative = differentiate(expr, wrt(d));
            static_assert(evaluate(derivative, at(d = 2.0)) == -1.0/4.0);
            expect(eq(evaluate(derivative, at(d = 2.0)), -1.0/4.0));
        }
        {
            constexpr function zero = differentiate(exp(a), wrt(b));
            static_assert(zero() == 0);
            expect(eq(zero(), 0.0));
        }
    };

    "derivative_expression_exp"_test = [] () {
        var a;
        var b;
        let mu;
        auto expr = exp((a + b)*b)*mu;
        {
            auto derivative = differentiate(expr, wrt(a));
            expect(eq(
                evaluate(derivative, at(a = 1.0, b = 2.0, mu = 3.0)),
                std::exp((1.0 + 2.0)*2.0)*3.0*2.0
            ));
        }
        {
            auto derivative = differentiate(expr, wrt(b));
            expect(eq(
                evaluate(derivative, at(a = 1.0, b = 2.0, mu = 3.0)),
                std::exp((1.0 + 2.0)*2.0)*3.0*(1.0 + 2.0 + 2.0)
            ));
        }
        {
            auto derivative = differentiate(expr, wrt(mu));
            expect(eq(
                evaluate(derivative, at(a = 1.0, b = 2.0, mu = 3.0)),
                std::exp((1.0 + 2.0)*2.0)
            ));
        }
    };

    "derivative_expression_sqrt"_test = [] () {
        var a;
        var b;
        let mu;
        auto expr = sqrt((a + b)*b)*mu;
        auto derivative = differentiate(expr, wrt(a));
        expect(eq(
            evaluate(derivative, at(a = 1.0, b = 2.0, mu = 3.0)),
            (1.0/std::sqrt((1.0 + 2.0)*2.0))*3.0*2.0
        ));
    };

    "bound_expression_back_propagate"_test = [] () {
        static constexpr var a;
        static constexpr let b;
        constexpr auto formula = ((a + b)*a).with(a = 2.0, b = 4.0);

        using exp_type = typename expression_type<std::remove_cvref_t<decltype(formula)>>::type;
        using bin_type = typename bindings_type<std::remove_cvref_t<decltype(formula)>>::type;
        static_assert(!std::is_lvalue_reference_v<exp_type>);
        static_assert(!std::is_lvalue_reference_v<bin_type>);

        static_assert(formula.template back_propagate<double>(wrt(a)).second[a] == 8.0);
        expect(eq(formula.template back_propagate<double>(wrt(a)).second[a], 8.0));
    };

    "bound_expression_referencing_back_propagate"_test = [] () {
        static constexpr var a;
        static constexpr let b;
        static constexpr auto expression = (a + b)*a;
        constexpr auto formula = expression.with(a = 2.0, b = 4.0);

        using exp_type = typename expression_type<std::remove_cvref_t<decltype(formula)>>::type;
        using bin_type = typename bindings_type<std::remove_cvref_t<decltype(formula)>>::type;
        static_assert(std::is_lvalue_reference_v<exp_type>);
        static_assert(!std::is_lvalue_reference_v<bin_type>);

        static_assert(formula.template back_propagate<double>(wrt(a)).second[a] == 8.0);
        expect(eq(formula.template back_propagate<double>(wrt(a)).second[a], 8.0));
    };

    "bound_expression_differentiate"_test = [] () {
        static constexpr var a;
        static constexpr let b;
        static constexpr auto formula = ((a + b)*a).with(a = 2.0, b = 4.0);
        constexpr auto derivative = formula.differentiate(wrt(a));

        using exp_type = typename expression_type<std::remove_cvref_t<decltype(derivative)>>::type;
        using bin_type = typename bindings_type<std::remove_cvref_t<decltype(derivative)>>::type;
        static_assert(!std::is_lvalue_reference_v<exp_type>);
        static_assert(std::is_lvalue_reference_v<bin_type>);

        static_assert(derivative.evaluate() == 8.0);
        expect(eq(derivative.evaluate(), 8.0));
    };

    "bound_expression_differentiate_on_temporary"_test = [] () {
        static constexpr var a;
        static constexpr let b;
        static constexpr auto expression = (a + b)*a;
        static constexpr auto derivative = expression.with(a = 2.0, b = 4.0).differentiate(wrt(a));

        using exp_type = typename expression_type<std::remove_cvref_t<decltype(derivative)>>::type;
        using bin_type = typename bindings_type<std::remove_cvref_t<decltype(derivative)>>::type;
        static_assert(!std::is_lvalue_reference_v<exp_type>);
        static_assert(!std::is_lvalue_reference_v<bin_type>);

        static_assert(derivative.evaluate() == 8.0);
        expect(eq(derivative.evaluate(), 8.0));
    };

    "function_differentiate"_test = [] () {
        var a;
        let b;
        var c;
        function f = (a + b)*a;
        function df_da = f.differentiate(wrt(a));
        expect(eq(df_da(a = 2.0, b = 4.0, c = 10.0), 2.0*2.0 + 4.0));
    };

    return EXIT_SUCCESS;
}
