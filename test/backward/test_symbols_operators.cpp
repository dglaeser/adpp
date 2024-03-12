#include <cstdlib>
#include <type_traits>

#include <boost/ut.hpp>

#include <adpp/backward/symbols.hpp>

// TODO: This include should not be necessary?
#include <adpp/backward/expression.hpp>

// TODO: simplify this? Required for "function"...
#include <adpp/backward/evaluate.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using adpp::backward::var;
using adpp::backward::cval;
using adpp::backward::value;
using adpp::backward::function;


template<typename T> struct is_value : std::false_type {};
template<typename T, auto _> struct is_value<adpp::backward::value<T, _>> : std::true_type {};

template<typename E>
struct value_stored_type {
    using _values = adpp::filtered_types_t<is_value, adpp::backward::symbols_t<E>>;
    static_assert(adpp::type_list_size_v<_values> == 1);
    using type = typename adpp::first_type_t<_values>::stored_type;
};


int main() {

    "var_times_scalar"_test = [] () {
        var a;
        {
            [[maybe_unused]] auto op_right = a*cval<1>;
            [[maybe_unused]] auto op_left = cval<1>*a;
        }
        {
            [[maybe_unused]] auto op_right = a + cval<1>;
            [[maybe_unused]] auto op_left = cval<1> + a;
        }
        {
            [[maybe_unused]] auto op_right = a - cval<1>;
            [[maybe_unused]] auto op_left = cval<1> - a;
        }
    };

    "expr_operation_with_owned_value"_test = [] () {
        var a;
        function f = a*2.0;
        static_assert(std::is_same_v<
            typename value_stored_type<std::remove_cvref_t<decltype(f)>>::type,
            double
        >);
        static_assert(std::is_same_v<decltype(f(a = 2)), double>);
        expect(eq(f(a = 2), 4));
    };

    "expr_operation_with_referenced_value"_test = [] () {
        double v = 2.0;
        var a;
        function f = a*v;
        static_assert(std::is_same_v<
            typename value_stored_type<std::remove_cvref_t<decltype(f)>>::type,
            double*
        >);
        static_assert(std::is_same_v<decltype(f(a = 2)), double>);
        expect(eq(f(a = 2), 4));

        v = 8.0;
        expect(eq(f(a = 2), 16));
    };

    return EXIT_SUCCESS;
}
