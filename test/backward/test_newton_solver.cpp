#include <cstdlib>

#include <iostream>
#include <boost/ut.hpp>

#include <cppad/backward/var.hpp>
#include <cppad/backward/let.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using cppad::backward::var;
using cppad::backward::let;
using cppad::backward::expression;

constexpr double newton_solve() {
    var x = 10.0;
    expression f = x*x*2.0 - 4.0;

    int it = 0;
    auto value = f.value();
    while (value > 1e-6 && it < 100) {
        x -= value/f.partial(x);
        value = f.value();
        ++it;
    }

    return value;
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

