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

    "expression_gradient"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        static constexpr let mu;
        constexpr auto expr = (a + b)*b*mu;
        constexpr auto gradient = grad(expr, at(a = 1.0, b = 2.0, mu = 3.0));
        static_assert(2.0*3.0 == gradient[a]);
        static_assert(5.0*3.0 == gradient[b]);
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

    "derivative_expression"_test = [] () {
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

    "derivative_expression_disappearing_variable"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        static constexpr let mu;
        static constexpr auto expr = cval<2>*a + b*mu;
        // we still have to provide all variables
        {
            constexpr auto derivative = differentiate(expr, wrt(a));
            static_assert(evaluate(derivative, at(a = 1.0, b = 2.0, mu = 3.0)) == 2.0);
        }
        {
            constexpr auto derivative = differentiate(expr, wrt(b));
            static_assert(evaluate(derivative, at(a = 1.0, b = 2.0, mu = 3.0)) == 3.0);
        }
        {
            constexpr auto derivative = differentiate(expr, wrt(mu));
            static_assert(evaluate(derivative, at(a = 1.0, b = 2.0, mu = 3.0)) == 2.0);
        }
    };

    "derivative_expression_complex"_test = [] () {
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

    return EXIT_SUCCESS;
}
