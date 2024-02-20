#include <cstdlib>

#include <boost/ut.hpp>
#include <cppad/function.hpp>

int main() {
    using boost::ut::literals::operator""_ul;
    using boost::ut::operator""_test;
    using boost::ut::expect;
    using boost::ut::throws;
    using boost::ut::eq;

    auto a = cppad::var(1);
    auto b = cppad::var(2);

    "independent_variables_size"_test = [&] () {
        expect(eq(cppad::IndependentVariables{}.size(), 0_ul));
        expect(eq(cppad::IndependentVariables{a}.size(), 1_ul));
        expect(eq(cppad::IndependentVariables{a, b}.size(), 2_ul));
    };

    "independent_variables_double_insertion_fails"_test = [&] () {
        expect(throws([&] () { cppad::IndependentVariables{a, a}; }));
    };

    return EXIT_SUCCESS;
}
