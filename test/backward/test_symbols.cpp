#include <cstdlib>
#include <type_traits>

#include <boost/ut.hpp>

#include <cppad/backward/symbols.hpp>

using boost::ut::operator""_test;

using cppad::backward::var;
using cppad::backward::let;
using cppad::backward::value_binder;

using cppad::dtype::real;
using cppad::dtype::integral;

template<typename S, typename V>
constexpr bool holds_reference(const value_binder<S, V>&) {
    return std::is_lvalue_reference_v<V>;
}

int main(int argc, char** argv) {

    "var_instances_are_unique"_test = [] () {
        var a;
        var b;
        static_assert(!std::is_same_v<decltype(a), decltype(b)>);

        var c = std::move(a);
        static_assert(std::is_same_v<decltype(a), decltype(c)>);
    };

    "let_instances_are_unique"_test = [] () {
        let a;
        let b;
        static_assert(!std::is_same_v<decltype(a), decltype(b)>);

        let c = std::move(a);
        static_assert(std::is_same_v<decltype(a), decltype(c)>);
    };

    "var_binder_owned_value"_test = [] () {
        constexpr var a;
        constexpr value_binder b = a.bind(22.0);
        constexpr value_binder c = a = 42.0;
        static_assert(!holds_reference(b));
        static_assert(!holds_reference(c));
        static_assert(std::is_same_v<decltype(b), decltype(c)>);
        static_assert(b.unwrap() == 22.0);
        static_assert(c.unwrap() == 42.0);
    };

    "let_binder_owned_value"_test = [] () {
        constexpr let a;
        constexpr value_binder b = a.bind(22.0);
        constexpr value_binder c = a = 42.0;
        static_assert(!holds_reference(b));
        static_assert(!holds_reference(c));
        static_assert(std::is_same_v<decltype(b), decltype(c)>);
        static_assert(b.unwrap() == 22.0);
        static_assert(c.unwrap() == 42.0);
    };

    "var_binder_referenced_value"_test = [] () {
        static constexpr double value = 42.0;
        constexpr var a;
        constexpr value_binder b = a.bind(value);
        constexpr value_binder c = a = value;
        static_assert(holds_reference(b));
        static_assert(holds_reference(c));
        static_assert(std::is_same_v<decltype(b), decltype(c)>);
        static_assert(b.unwrap() == 42.0);
        static_assert(c.unwrap() == 42.0);
    };

    "let_binder_referenced_value"_test = [] () {
        static constexpr double value = 42.0;
        constexpr let a;
        constexpr value_binder b = a.bind(value);
        constexpr value_binder c = a = value;
        static_assert(holds_reference(b));
        static_assert(holds_reference(c));
        static_assert(std::is_same_v<decltype(b), decltype(c)>);
        static_assert(b.unwrap() == 42.0);
        static_assert(c.unwrap() == 42.0);
    };

    "var_binder_constrained_on_real_numbers"_test = [] () {
        constexpr var<real> a;
        constexpr auto b = a = 22.0;
        static_assert(!holds_reference(b));
        static_assert(b.unwrap() == 22.0);
    };

    "let_binder_constrained_on_real_numbers"_test = [] () {
        constexpr let<real> a;
        constexpr auto b = a = 22.0;
        static_assert(!holds_reference(b));
        static_assert(b.unwrap() == 22.0);
    };

    "var_binder_constrained_on_integer_numbers"_test = [] () {
        constexpr var<integral> a;
        constexpr auto b = a = 42;
        static_assert(!holds_reference(b));
        static_assert(b.unwrap() == 42);
    };

    "let_binder_constrained_on_integer_numbers"_test = [] () {
        constexpr let<integral> a;
        constexpr auto b = a = 42;
        static_assert(!holds_reference(b));
        static_assert(b.unwrap() == 42);
    };

    "let_binder_constrained_on_scalar_type"_test = [] () {
        constexpr let<int> a;
        constexpr auto b = a = 42;
        static_assert(!holds_reference(b));
        static_assert(b.unwrap() == 42);
    };

    return EXIT_SUCCESS;
}
