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

int main(int argc, char** argv) {

    "var_types_are_unique"_test = [&] () {
        var a = 1.0;
        var b = 2.0;
        static_assert(!std::same_as<decltype(a), decltype(b)>);

        var<int> c;
        var<int> d;
        static_assert(!std::same_as<decltype(c), decltype(d)>);
    };

    "var_declaration"_test = [&] () {
        var<double> a;
        var b = 1;
        b = 5;

        static_assert(cppad::is_variable_v<decltype(a)>);
        static_assert(sizeof(a) == sizeof(double));
        static_assert(sizeof(b) == sizeof(int));
        static_assert(!std::same_as<decltype(a), decltype(b)>);

        expect(eq(a.value(), cppad::undefined_value_v<double>));
        expect(eq(double(a), cppad::undefined_value_v<double>));
        expect(eq(b.value(), 5));
        expect(eq(int(b), 5));
        expect(eq(derivative_of(a, wrt(a)), 1.0));
        expect(eq(derivative_of(a, wrt(b)), 0.0));
    };

    "var_assignment_ctor"_test = [] {
        var a = 1;
        var b = 2.0;
        expect(eq(a.value(), int{1}));
        expect(eq(b.value(), double{2}));
    };

    "var_name_binding"_test = [] () {
        constexpr auto assert = [] <typename T> (const cppad::backward::named_expression<T>&) {
            static_assert(cppad::is_variable_v<T>);
        };

        static constexpr var v = 2;
        constexpr auto named = v |= "myvar";
        assert(named);
        static_assert(std::string_view{named.name()} == std::string_view{"myvar"});
    };

    "var_at_compile_time"_test = [] () {
        constexpr var v = 1;
        constexpr var b = 1;
        static_assert(!std::is_same_v<decltype(v), decltype(b)>);
        static_assert(v.value() == 1);
        static_assert(derivative_of(v, wrt(v)) == 1);
        static_assert(derivative_of(v, wrt(b)) == 0);
    };

    "var_to_expression"_test = [] () {
        static_assert(cppad::concepts::expression<cppad::backward::var<int>>);
        static_assert(cppad::concepts::into_expression<cppad::backward::var<int>>);

        const var a = 42.0;
        static_assert(std::is_same_v<decltype(cppad::to_expression(a)), decltype(a)&>);
        expect(cppad::is_same_object(a, cppad::to_expression(a)));
    };

    "let_declaration"_test = [] () {
        let a{1.0};
        let b = 5.0;

        static_assert(cppad::is_constant_v<decltype(a)>);
        static_assert(sizeof(a) == sizeof(double));
        static_assert(sizeof(b) == sizeof(double));
        static_assert(!std::same_as<decltype(a), decltype(b)>);

        expect(eq(b.value(), double{5}));
        a = 1.0;
        expect(eq(a.value(), 1.0));
        expect(eq(double(a), 1.0));
        expect(eq(b.value(), 5.0));
        expect(eq(double(b), 5.0));
        expect(eq(derivative_of(a, wrt(a)), 0.0));
        expect(eq(derivative_of(a, wrt(b)), 0.0));
    };

    "let_to_expression"_test = [] () {
        double a = 1.0;
        auto c = cppad::to_expression(a);
        static_assert(cppad::is_constant_v<decltype(c)>);
    };

    "let_assignmet_ctor"_test = [] () {
        let a = 1;
        let b = 2.0;
        expect(eq(a.value(), int{1}));
        expect(eq(b.value(), double{2}));
    };

    return EXIT_SUCCESS;
}
