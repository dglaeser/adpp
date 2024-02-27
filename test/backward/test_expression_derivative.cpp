#include <cstdlib>

#include <boost/ut.hpp>

#include <cppad/backward/symbols.hpp>
#include <cppad/backward/differentiate.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using cppad::backward::var;
using cppad::backward::let;


int main(int argc, char** argv) {

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
        auto expr = std::exp((a + b)*b);
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

    return EXIT_SUCCESS;
}
