#include <cstdlib>
#include <cstddef>
#include <string_view>

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

    "variable_as_expression"_test = [] () {
        static_assert(cppad::concepts::Expression<cppad::Variable<int>>);
        static_assert(cppad::concepts::IntoExpression<cppad::Variable<int>>);

        const auto a = cppad::var(42.0);
        static_assert(std::is_same_v<decltype(cppad::as_expression(a)), decltype(a)&>);
        expect(cppad::detail::is_same_object(a, cppad::as_expression(a)));
    };

    "variable_at_compile_time"_test = [] () {
        constexpr auto v = cppad::var(1);
        constexpr auto b = cppad::var(1);
        static_assert(!std::is_same_v<decltype(v), decltype(b)>);
        static_assert(v.value() == 1);
        static_assert(v.partial(v) == 1);
        static_assert(v.partial(b) == 0);
    };

    "variable_name_assignment"_test = [] () {
        constexpr auto references = [] <typename T> (const cppad::NamedVariable<T>&) {
            return std::is_lvalue_reference_v<T>;
        };

        static constexpr auto v = cppad::var(2);
        constexpr auto named = v |= "myvar";
        static_assert(references(named) == true);
        static_assert(std::string_view{named.name()} == std::string_view{"myvar"});
    };

    return EXIT_SUCCESS;
}
