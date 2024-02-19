#include <cstdlib>
#include <cstddef>

#include <boost/ut.hpp>
#include <cppad/constant.hpp>


int main() {
    using boost::ut::operator""_test;
    using boost::ut::expect;
    using boost::ut::eq;

    "constant_value"_test = [] () {
        auto a = cppad::constant(42.0);
        expect(eq(a.value(), 42.0));
    };

    "constant_value_int"_test = [] () {
        auto a = cppad::constant(int{42});
        expect(eq(a.value(), int{42}));
    };

    "constant_value_float"_test = [] () {
        auto a = cppad::constant(float{42});
        expect(eq(a.value(), float{42}));
    };

    "constant_value_uint8"_test = [] () {
        auto a = cppad::constant(std::uint8_t{42});
        expect(eq(a.value(), std::uint8_t{42}));
    };

    "constant_value_from_temporary"_test = [] () {
        {
            auto a = cppad::constant(double{42.0});
            expect(eq(a.value(), 42.0));
            static_assert(std::is_same_v<decltype(a.value()), double&>);
        }
        {
            cppad::Constant a{double{42.0}};
            expect(eq(a.value(), 42.0));
            static_assert(std::is_same_v<decltype(a.value()), double&>);
        }
    };

    "constant_value_from_temporary_const"_test = [] () {
        {
            const auto a = cppad::constant(double{42.0});
            expect(eq(a.value(), 42.0));
            static_assert(std::is_same_v<decltype(a.value()), const double&>);
        }
        {
            const cppad::Constant a{double{42.0}};
            expect(eq(a.value(), 42.0));
            static_assert(std::is_same_v<decltype(a.value()), const double&>);
        }
    };

    "constant_value_from_reference_takes_ownership"_test = [] () {
        {
            double value = 42.0;
            auto a = cppad::constant(value);
            value *= 2.0;
            expect(eq(a.value(), 42.0));
            static_assert(std::is_same_v<decltype(a.value()), double&>);
        }
        {
            double value = 42.0;
            auto a = cppad::constant(value);
            value *= 2.0;
            expect(eq(a.value(), 42.0));
            static_assert(std::is_same_v<decltype(a.value()), double&>);
        }
    };

    "constant_value_from_reference_takes_ownership_const"_test = [] () {
        {
            double value = 42.0;
            const auto a = cppad::constant(value);
            value *= 2.0;
            expect(eq(a.value(), 42.0));
            static_assert(std::is_same_v<decltype(a.value()), const double&>);
        }
        {
            double value = 42.0;
            const auto a = cppad::constant(value);
            value *= 2.0;
            expect(eq(a.value(), 42.0));
            static_assert(std::is_same_v<decltype(a.value()), const double&>);
        }
    };

    return EXIT_SUCCESS;
}
