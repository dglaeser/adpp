#include <cstdlib>
#include <type_traits>
#include <sstream>
#include <array>

#include <boost/ut.hpp>

#include <adpp/backward/tensor.hpp>
#include <adpp/backward/operators.hpp>
#include <adpp/backward/evaluate.hpp>
#include <adpp/backward/differentiate.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using adpp::shape;
using adpp::length;
using adpp::backward::var;
using adpp::backward::cval;
using adpp::backward::vec;
using adpp::backward::vector;
using adpp::backward::tensor;
using adpp::backward::vector_expression;
using adpp::backward::tensor_expression;
using adpp::backward::tensor_value_binder;
using adpp::backward::value_binder;

template<typename T>
struct is_vec_value_binder : std::false_type {};
template<typename E, typename T>
struct is_vec_value_binder<tensor_value_binder<E, T>> : std::true_type {};

template<typename T>
struct bound_value_type;
template<typename E, typename T>
struct bound_value_type<tensor_value_binder<E, T>> : std::type_identity<T> {};

template<typename jacobian>
constexpr auto invert_2x2(jacobian&& J) {
    using adpp::md_index;
    const auto det = 1.0/(J[md_index<0, 0>]*J[md_index<1, 1>] - J[md_index<0, 1>]*J[md_index<1, 0>]);
    J.scale_with(det);
    std::swap(J[md_index<0, 0>], J[md_index<1, 1>]);
    J[md_index<0, 1>] *= -1;
    J[md_index<1, 0>] *= -1;
    return J;
}

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
        vector v(length<3>);
        static constexpr auto result = evaluate(v, at(v = std::array{0, 1, 2}));
        static_assert(std::is_same_v<
            std::remove_cvref_t<decltype(result)>,
            adpp::md_array<int, adpp::md_shape<3, 1>{}>
        >);
        static_assert(result[0] == 0);
        static_assert(result[1] == 1);
        static_assert(result[2] == 2);
        expect(eq(result[0], 0));
        expect(eq(result[1], 1));
        expect(eq(result[2], 2));
    };

    "vec_binding_by_reference"_test = [] () {
        vector v(length<3>);
        std::array<double, 3> values{42, 43, 44};
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
        using adpp::index;

        vector v(length<3>);
        constexpr vector_expression e = {
            v[index<0>]*v[index<1>],
            v[index<1>]*v[index<2>],
            v[index<2>]*v[index<0>]
        };
        static constexpr auto result = evaluate(e, at(v = {42, 2, 10}));
        expect(eq(result[0],  84));
        expect(eq(result[1],  20));
        expect(eq(result[2],  420));
    };

    "vec_expression_scaling"_test = [] () {
        vector v(length<3>);
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
        {
            vector v(length<3>);
            vector v2(length<3>);
            constexpr auto e = v.dot(v2);
            static_assert(evaluate(e, at(v = {1, 2, 3}, v2 = {1, 2, 3})) == 14);
        }
        {  // same with using vec
            vec<3> v;
            vec<3> v2;
            constexpr auto e = v.dot(v2);
            static_assert(evaluate(e, at(v = {1, 2, 3}, v2 = {1, 2, 3})) == 14);
        }
        {
            vector v(length<3>);
            auto e = v.dot(v);
            expect(evaluate(e, at(v = {1, 2, 3})) == 14);
        }
        {  // same with using vec
            vec<3> v;
            auto e = v.dot(v);
            expect(evaluate(e, at(v = {1, 2, 3})) == 14);
        }
    };

    "vec_expression_l2_norm"_test = [] () {
        {
            vector v(length<3>);
            auto e = v.l2_norm();
            expect(eq(evaluate(e, at(v = {1, 2, 3})), std::sqrt(14)));
        }
    };

    "vec_expression_l2_norm_2"_test = [] () {
        {
            vector v(length<3>);
            constexpr auto e = v.l2_norm_squared();
            static_assert(evaluate(e, at(v = {1, 2, 3})) == 14);
        }
        {
            vector v(length<3>);
            auto e = v.l2_norm_squared();
            expect(evaluate(e, at(v = {1, 2, 3})) == 14);
        }
    };

    "vec_io"_test = [] () {
        vector v(length<3>);
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

    "tensor_elements_expression"_test = [] () {
        tensor t{shape<2, 2>};
        std::ostringstream s;
        s << t.with(t = {"t11", "t12", "t21", "t22"});
        expect(eq(s.str(), std::string{"[t11, t12 // t21, t22]"}));
    };

    "tensor_elements_scaling"_test = [] () {
        constexpr tensor t{shape<2, 2>};
        constexpr auto expr = t.scaled_with(cval<3>);
        constexpr auto result = evaluate(expr, at(t = {1, 2, 3, 4}));
        static_assert(result[0, 0] == 3);
        static_assert(result[0, 1] == 6);
        static_assert(result[1, 0] == 9);
        static_assert(result[1, 1] == 12);
    };

    "tensor_expression"_test = [] () {
        var x;
        var y;
        constexpr tensor_expression e = {adpp::md_shape<2, 2, 1>{}, x, x*y, y*x, y};
        constexpr auto result = evaluate(e, at(x = 1, y = 2));
        static_assert(std::is_same_v<
            std::remove_cvref_t<decltype(result)>,
            adpp::md_array<int, adpp::md_shape<2, 2, 1>{}>
        >);
        static_assert(result[0, 0, 0] == 1);
        static_assert(result[0, 1, 0] == 2);
        static_assert(result[1, 0, 0] == 2);
        static_assert(result[1, 1, 0] == 2);
    };

    "tensor_expression_scaling"_test = [] () {
        var x;
        var y;
        constexpr tensor_expression e = {adpp::md_shape<2, 2>{}, x, x*y, y*x, y};
        constexpr auto scaled = e.scaled_with(cval<2>);
        constexpr auto result = evaluate(scaled, at(x = 1, y = 2));
        static_assert(result[0, 0] == 2);
        static_assert(result[0, 1] == 4);
        static_assert(result[1, 0] == 4);
        static_assert(result[1, 1] == 4);
    };

    "tensor_expression_vector_dot"_test = [] () {
        var x;
        var y;
        constexpr tensor_expression t = {adpp::md_shape<2, 2>{}, x, x*y, y*x, y};
        constexpr vector_expression v = {x, y};
        constexpr auto expr = t.apply_to(v);
        constexpr auto result = evaluate(expr, at(x = 1, y = 2));
        static_assert(expr.shape == adpp::md_shape<2, 1>{});
        static_assert(result[0] == 1*1 + 1*2*2);
        static_assert(result[1] == 1*2*1 + 2*2);
    };

    "tensor_expression_tensor_apply"_test = [] () {
        using namespace adpp::indices;
        var x;
        var y;
        constexpr tensor_expression t0 = {adpp::md_shape<2, 2>{}, x, x*y, y*x, y};
        constexpr tensor_expression t1 = {adpp::md_shape<2, 2>{}, x, x+y, y*y, y};
        constexpr auto expr = t0.apply_to(t1);
        static_assert(std::is_same_v<
            decltype(expr[adpp::md_index<_0, _0>]),
            decltype(x*x + (x*y)*(y*y))
        >);
        static_assert(std::is_same_v<
            decltype(expr[adpp::md_index<_0, _1>]),
            decltype(x*(x+y) + (x*y)*y)
        >);
        static_assert(std::is_same_v<
            decltype(expr[adpp::md_index<_1, _0>]),
            decltype((y*x)*x + y*(y*y))
        >);
        static_assert(std::is_same_v<
            decltype(expr[adpp::md_index<_1, _1>]),
            decltype((y*x)*(x+y) + y*y)
        >);

        constexpr auto result = evaluate(expr, at(x = 1, y = 2));
        static_assert(expr.shape == adpp::md_shape<2, 2>{});
        static_assert(result[0, 0] == 1*1 + 1*2*(2*2));
        static_assert(result[0, 1] == 1*(1 + 2) + 1*2*2);
        static_assert(result[1, 0] == (1*2)*1 + 2*2*2);
        static_assert(result[1, 1] == (2*1)*(1+2) + 2*2);
    };

    "md_tensor_expression_tensor_apply"_test = [] () {
        tensor t0{shape<1, 2, 3>};
        tensor t1{shape<3, 2>};
        constexpr auto expr = t0.apply_to(t1);
        static_assert(expr.shape == adpp::md_shape<1, 2, 2>{});
        constexpr auto result = evaluate(expr, at(t0 = {1, 2, 3, 4, 5, 6}, t1 = {1, 2, 3, 4, 5, 6}));
        static_assert(result[0, 0, 0] == 1*1 + 2*3 + 3*5);
        static_assert(result[0, 0, 1] == 1*2 + 2*4 + 3*6);
        static_assert(result[0, 1, 0] == 4*1 + 5*3 + 6*5);
        static_assert(result[0, 1, 1] == 4*2 + 5*4 + 6*6);
    };

    "md_tensor_expression_tensor_apply_same_size"_test = [] () {
        tensor t0{shape<2, 2>};
        tensor t1{shape<2, 2>};
        constexpr auto expr = t0.apply_to(t1);
        static_assert(expr.shape == adpp::md_shape<2, 2>{});
        constexpr auto result = evaluate(expr, at(t0 = {1, 2, 3, 4}, t1 = {1, 2, 3, 4}));
        static_assert(result[0, 0] == 1*1 + 2*3);
        static_assert(result[0, 1] == 1*2 + 2*4);
        static_assert(result[1, 0] == 3*1 + 4*3);
        static_assert(result[1, 1] == 3*2 + 4*4);
    };

    "md_tensor_expression_apply_with_scalar_result"_test = [] () {
        tensor t0{shape<1, 3>};
        tensor t1{shape<3, 1>};
        constexpr auto expr = t0.apply_to(t1);
        constexpr auto scalar_expr = t0.apply_to(t1).as_scalar();
        static_assert(expr.shape == adpp::md_shape<1, 1>{});
        static_assert(evaluate(expr, at(t0 = {1, 2, 3}, t1 = {2, 3, 4}))[0] == 20);
        static_assert(evaluate(scalar_expr, at(t0 = {1, 2, 3}, t1 = {2, 3, 4})) == 20);
    };

    "vector_expression_jacobian"_test = [] () {
        using namespace adpp::indices;

        var x;
        var y;
        constexpr vector_expression expr{x + y, x*y, x + cval<2>*y};
        constexpr auto jac = expr.jacobian(wrt(x, y), at(x = 1, y = 2));
        static_assert(std::is_lvalue_reference_v<decltype(jac[_0, _0])>);

        static_assert(jac[_0, x] == 1);
        static_assert(jac[_0, _0] == 1);

        static_assert(jac[_0, y] == 1);
        static_assert(jac[_0, _1] == 1);

        static_assert(jac[_1, x] == 2);
        static_assert(jac[_1, _0] == 2);

        static_assert(jac[_1, y] == 1);
        static_assert(jac[_1, _1] == 1);

        static_assert(jac[_2, x] == 1);
        static_assert(jac[_2, _0] == 1);

        static_assert(jac[_2, _1] == 2);
        static_assert(jac[_2, _1] == 2);

        constexpr auto v = jac.apply_to(std::array<double, 2>{1.0, 2.0});
        static_assert(v[0] == 3.0);
        static_assert(v[1] == 4.0);
        static_assert(v[2] == 5.0);
    };

    "vector_expression_jacobian_implicit_vars"_test = [] () {
        using namespace adpp::indices;

        var x;
        var y;
        vector_expression expr{x*x, x*y, x + y};
        auto jac = expr.jacobian(at(x = 1, y = 2));
        expect(eq(jac[_0, x], 2.0));
        expect(eq(jac[_1, x], 2.0));
        expect(eq(jac[_2, x], 1.0));

        expect(eq(jac[_0, y], 0.0));
        expect(eq(jac[_1, y], 1.0));
        expect(eq(jac[_2, y], 1.0));
    };

    "vector_expression_jacobian_modifiable_entries"_test = [] () {
        using namespace adpp::indices;

        var x;
        var y;
        vector_expression expr{x + y, x*y, x + cval<2>*y};
        auto jac = expr.jacobian(wrt(x, y), at(x = 1, y = 2));

        jac[_0, _0] = 0.0;
        jac[_0, _0] = 42.0;
        expect(eq(jac[_0, _0], 42.0));
    };

    "md_newton"_test = [] () {
        using namespace adpp::indices;
        static constexpr vec<2> v;
        constexpr vector_expression pde{
            cval<0.5>*v[_0]*v[_0] + v[_1],
            cval<5>*v[_0] - v[_1] + cval<2>
        };

        const auto solve = [pde] () {
            int i = 0;
            std::array x{5.0, 1.0};
            auto res = pde.evaluate(at(v = x));
            while (res.l2_norm_squared() > 1e-12 && i < 20) {
                auto J = pde.jacobian(wrt(v.vars()), at(v = x));
                invert_2x2(J).scaled_with(-1.0).add_apply_to(res, x);
                res = pde.evaluate(at(v = x));
                i++;
            }

            return x;
        };

        constexpr auto x = solve();
        static_assert(pde.evaluate(at(v = x))[0] < 1e-6);
        static_assert(pde.evaluate(at(v = x))[1] < 1e-6);
    };

    "md_newton_from_jacobian_expression"_test = [] () {
        using namespace adpp::indices;
        using adpp::md_index;
        static constexpr vec<2> v;
        static constexpr vector_expression pde{
            cval<0.5>*v[_0]*v[_0] + v[_1],
            cval<5>*v[_0]*v[_1] + cval<2>
        };
        static constexpr auto J = pde.jacobian_expression(wrt(v.vars()));

        const auto solve = [] () {
            int i = 0;
            std::array x{5.0, 1.0};
            auto res = pde.evaluate(at(v = x));
            while (res.l2_norm_squared() > 1e-12 && i < 20) {
                invert_2x2(J.evaluate(at(v = x))).scaled_with(-1.0).add_apply_to(res, x);
                res = pde.evaluate(at(v = x));
                i++;
            }

            return x;
        };

        constexpr auto x = solve();
        static_assert(pde.evaluate(at(v = x))[0] < 1e-6);
        static_assert(pde.evaluate(at(v = x))[1] < 1e-6);
    };

    return EXIT_SUCCESS;
}
