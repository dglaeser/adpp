#include <cstdlib>

#include <iostream>
#include <boost/ut.hpp>

#include <cppad/backward/symbols.hpp>
#include <cppad/backward/evaluate.hpp>
#include <cppad/backward/differentiate.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using cppad::backward::var;
using cppad::backward::let;
using cppad::backward::expression;

constexpr double newton_solve() {
    var x;
    expression f = x*x*2.0 - 4.0;

    int it = 0;
    double solution = 10.0;
    double residual = evaluate(f, at(x = solution));
    while (residual > 1e-6 && it < 100) {
        solution -= residual/derivative_of(f, wrt(x), at(x = solution));
        residual = evaluate(f, at(x = solution));
        ++it;
    }

    return residual;
}

int main() {
    using boost::ut::operator""_test;
    using boost::ut::expect;
    using boost::ut::eq;

    "newton_solver"_test = [] () {
        static_assert(newton_solve() < 1e-6);
    };

    return EXIT_SUCCESS;
}
