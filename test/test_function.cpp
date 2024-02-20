#include <cstdlib>

#include <boost/ut.hpp>
#include <cppad/function.hpp>

int main() {
    using boost::ut::operator""_test;
    using boost::ut::expect;
    using boost::ut::eq;

    auto a = cppad::var(1);
    auto b = cppad::var(2);

    "independent_variables_size"_test = [&] () {
        static_assert(cppad::IndependentVariables{}.size() == 0);
        static_assert(cppad::IndependentVariables{a}.size() == 1);
        static_assert(cppad::IndependentVariables{a, b}.size() == 2);
    };

    return EXIT_SUCCESS;
}
