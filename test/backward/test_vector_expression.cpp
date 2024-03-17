#include <cstdlib>
#include <type_traits>
#include <sstream>

#include <boost/ut.hpp>

#include <adpp/backward/vector.hpp>
#include <adpp/backward/evaluate.hpp>
#include <adpp/backward/operators.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using adpp::ic;
using adpp::backward::var;
using adpp::backward::vec;
using adpp::backward::cval;
using adpp::backward::vector_expression;
using adpp::backward::vector_value_binder;
using adpp::backward::value_binder;

template<typename T>
struct is_vec_value_binder : std::false_type {};
template<typename E, typename T>
struct is_vec_value_binder<vector_value_binder<E, T>> : std::true_type {};

template<typename T>
struct bound_value_type;
template<typename E, typename T>
struct bound_value_type<vector_value_binder<E, T>> : std::type_identity<T> {};

int main() {

    "vector_expression"_test = [] () {
        var x;
        var y;
        vector_expression v = {x, y};

        using X = std::remove_cvref_t<decltype(x)>;
        using Y = std::remove_cvref_t<decltype(y)>;
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(v)>, vector_expression<X, Y>>);

        auto binding = v = {1, 2};
        using binding_t = std::remove_cvref_t<decltype(binding)>;
        static_assert(is_vec_value_binder<binding_t>::value);
        static_assert(std::is_same_v<typename bound_value_type<binding_t>::type, std::array<int, 2>>);

        auto sub_binders = binding.sub_binders();
        using sub_binders_t = std::remove_cvref_t<decltype(sub_binders)>;
        static_assert(std::is_same_v<sub_binders_t, std::tuple<
            value_binder<X, const int&>,
            value_binder<Y, const int&>
        >>);
    };

    "vec"_test = [] () {
        vec<3> v;
        static constexpr auto result = evaluate(v, at(v = std::array{0, 1, 2}));
        static_assert(adpp::backward::is_symbol_v<std::remove_cvref_t<decltype(v)>>);
        static_assert(std::is_same_v<
            std::remove_cvref_t<decltype(result)>,
            std::array<int, 3>
        >);
        static_assert(result[0] == 0);
        static_assert(result[1] == 1);
        static_assert(result[2] == 2);
        expect(eq(result[0], 0));
        expect(eq(result[1], 1));
        expect(eq(result[2], 2));
    };

    "vec_binding_by_reference"_test = [] () {
        vec<3> v;
        std::array<double, 3> values{42, 43, 44};
        // TODO: v*2 does not yield nice error messages.
        // The overloaded operators should be deactivated for vectors? Or properly implemented?
        auto expr = v.scaled_with(cval<2>);
        expect(std::ranges::equal(
            evaluate(expr, at(v = values)),
            std::array{84, 86, 88}
        ));

        values[0] = 13;
        expect(std::ranges::equal(
            evaluate(expr, at(v = values)),
            std::array{26, 86, 88}
        ));
    };

    "vec_expression"_test = [] () {
        vec<3> v;
        constexpr vector_expression e = {
            v[ic<0>]*v[ic<1>],
            v[ic<1>]*v[ic<2>],
            v[ic<2>]*v[ic<0>]
        };
        static constexpr auto result = evaluate(e, at(v = {42, 2, 10}));
        expect(eq(result[0],  84));
        expect(eq(result[1],  20));
        expect(eq(result[2],  420));
    };

    "vec_expression_scaling"_test = [] () {
        vec<3> v;
        {
            constexpr auto e = v.scaled_with(adpp::backward::cval<3>);
            static_assert(std::ranges::equal(
                evaluate(e, at(v = {1, 2, 3})),
                std::array{3, 6, 9}
            ));
        }
        {
            auto e = v.scaled_with(3);
            expect(std::ranges::equal(
                evaluate(e, at(v = {1, 2, 3})),
                std::array{3, 6, 9}
            ));
        }
    };

    "vec_expression_dot"_test = [] () {
        vec<3> v;
        constexpr auto e = v.dot(v);
        static_assert(evaluate(e, at(v = {1, 2, 3})) == 14);
    };

    "vec_io"_test = [] () {
        vec<3> v;
        std::ostringstream s;
        s << v.with(v = {"vx", "vy", "vz"});
        expect(eq(s.str(), std::string{"[vx, vy, vz]"}));
    };

    "vec_elements_expression"_test = [] () {
        vec<3> v;
        std::ostringstream s;
        s << v.with(v = {"vx", "vy", "vz"});
        expect(eq(s.str(), std::string{"[vx, vy, vz]"}));
    };

    return EXIT_SUCCESS;
}
