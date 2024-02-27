#include <cstdlib>
#include <iostream>
#include <type_traits>

#include <boost/ut.hpp>

#include <cppad/backward/symbols.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using cppad::backward::var;
using cppad::backward::value_binder;

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

    "var_binder_owned_value"_test = [] () {
        var a;
        value_binder b = a.bind(1.0);
        static_assert(!holds_reference(b));
    };

    "var_binder_referenced_value"_test = [] () {
        var a;
        double value = 1.0;
        value_binder b = a.bind(value);
        static_assert(holds_reference(b));
    };

    return EXIT_SUCCESS;
}
