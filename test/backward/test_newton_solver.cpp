#include <cstdlib>

#include <boost/ut.hpp>

#include <adpp/backward/symbols.hpp>
#include <adpp/backward/operators.hpp>
#include <adpp/backward/evaluate.hpp>
#include <adpp/backward/differentiate.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::lt;

using adpp::backward::var;
using adpp::backward::cval;

using adpp::backward::bind;
using adpp::backward::function;

constexpr double newton_solve() {
    double solution = 10.0;

    var x;
    function f = x*x*cval<2.0> - cval<4.0>;
    function df_dx = f.differentiate(wrt(x));

    const auto args = bind(x = solution);
    double residual = f(args);
    int it = 0; while (residual > 1e-6 && it < 100) {
        solution -= residual/df_dx(args);
        residual = f(args);
        ++it;
    }

    return residual;
}

int main() {
    using boost::ut::operator""_test;
    using boost::ut::expect;
    using boost::ut::lt;

    "newton_solver"_test = [] () {
        static_assert(newton_solve()*newton_solve() < 1e-12);
        expect(lt(newton_solve()*newton_solve(), 1e-12));
    };

    return EXIT_SUCCESS;
}
