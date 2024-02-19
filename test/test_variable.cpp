#include <cstdlib>
#include <cstddef>

#include <boost/ut.hpp>
#include <cppad/variable.hpp>

int main() {
    using boost::ut::operator""_test;
    using boost::ut::expect;
    using boost::ut::eq;

    "variable_value"_test = [] () {
        auto a = cppad::var(42.0);
        expect(eq(a.value(), 42.0));
    };

    "variable_value_int"_test = [] () {
        auto a = cppad::var(int{42});
        expect(eq(a.value(), int{42}));
    };

    "variable_value_float"_test = [] () {
        auto a = cppad::var(float{42});
        expect(eq(a.value(), float{42}));
    };

    "variable_value_uint8"_test = [] () {
        auto a = cppad::var(std::uint8_t{42});
        expect(eq(a.value(), std::uint8_t{42}));
    };

    "variable_value_from_temporary"_test = [] () {
        auto a = cppad::var(double{42.0});
        expect(eq(a.value(), 42.0));
        static_assert(std::is_same_v<decltype(a.value()), double&>);
    };

    "variable_value_from_temporary_const"_test = [] () {
        const auto a = cppad::var(double{42.0});
        expect(eq(a.value(), 42.0));
        static_assert(std::is_same_v<decltype(a.value()), const double&>);
    };

    "variable_value_from_reference"_test = [] () {
        double value = 42.0;
        auto a = cppad::var(value);
        value *= 2.0;
        expect(eq(a.value(), 42.0));
        static_assert(std::is_same_v<decltype(a.value()), double&>);
    };

    "variable_value_from_reference_const"_test = [] () {
        double value = 42.0;
        const auto a = cppad::var(value);
        value *= 2.0;
        expect(eq(a.value(), 42.0));
        static_assert(std::is_same_v<decltype(a.value()), const double&>);
    };

    "variable_self_derivative"_test = [] () {
        const auto a = cppad::var(42.0);
        expect(eq(a.partial(a), 1.0));
    };

    "variable_other_derivative"_test = [] () {
        const auto a = cppad::var(42.0);
        const auto other = cppad::var(20.0);
        expect(eq(a.partial(other), 0.0));
    };

    return EXIT_SUCCESS;
}
