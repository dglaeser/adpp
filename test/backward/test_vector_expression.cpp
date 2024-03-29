#include <cstdlib>
#include <type_traits>
#include <sstream>
#include <array>

#include <boost/ut.hpp>

#include <adpp/backward/vector.hpp>
#include <adpp/backward/operators.hpp>
#include <adpp/backward/evaluate.hpp>
#include <adpp/backward/differentiate.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;
using boost::ut::le;

using adpp::shape;
using adpp::backward::var;
using adpp::backward::vec;
using adpp::backward::cval;
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
        static_assert(std::is_same_v<
            std::remove_cvref_t<decltype(result)>,
            adpp::backward::md_array<int, adpp::md_shape<3, 1>{}>
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

        vec<3> v;
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
        vec<3> v;
        {
            constexpr auto e = v.scaled_with(adpp::backward::cval<3>);
            static_assert(std::ranges::equal(
                evaluate(e, at(v = {1, 2, 3})),
                std::array{3, 6, 9}
            ));
        }
        {
            constexpr auto e = v*cval<3>;
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
        {
            auto e = v*3;
            expect(std::ranges::equal(
                evaluate(e, at(v = {1, 2, 3})),
                std::array{3, 6, 9}
            ));
        }
    };

    "vec_expression_dot"_test = [] () {
        {
            vec<3> v;
            vec<3> v2;
            constexpr auto e = v.dot(v2);
            static_assert(evaluate(e, at(v = {1, 2, 3}, v2 = {1, 2, 3})) == 14);
        }
        {
            vec<3> v;
            constexpr auto e = v*v;
            static_assert(evaluate(e, at(v = {1, 2, 3})) == 14);
        }
        {
            vec<3> v;
            auto e = v.dot(v);
            expect(evaluate(e, at(v = {1, 2, 3})) == 14);
        }
    };

    "vec_expression_l2_norm"_test = [] () {
        {
            vec<3> v;
            auto e = v.l2_norm();
            expect(eq(evaluate(e, at(v = {1, 2, 3})), std::sqrt(14)));
        }
    };

    "vec_expression_l2_norm_2"_test = [] () {
        {
            vec<3> v;
            constexpr auto e = v.l2_norm_squared();
            static_assert(evaluate(e, at(v = {1, 2, 3})) == 14);
        }
        {
            vec<3> v;
            auto e = v.l2_norm_squared();
            expect(evaluate(e, at(v = {1, 2, 3})) == 14);
        }
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

    "tensor_elements_expression"_test = [] () {
        tensor t{shape<2, 2>};
        std::ostringstream s;
        s << t.with(t = {"t11", "t12", "t21", "t22"});
        expect(eq(s.str(), std::string{"[t11, t12 // t21, t22]"}));
    };

    "tensor_elements_scaling"_test = [] () {
        constexpr tensor t{shape<2, 2>};
        constexpr auto expr = t*cval<3>;
        constexpr auto result = evaluate(expr, at(t = {1, 2, 3, 4}));
        static_assert(result[0, 0] == 3);
        static_assert(result[0, 1] == 6);
        static_assert(result[1, 0] == 9);
        static_assert(result[1, 1] == 12);
    };

    "tensor_expression"_test = [] () {
        var x;
        var y;
        constexpr tensor_expression e = {adpp::md_shape<2, 2>{}, x, x*y, y*x, y};
        constexpr auto result = evaluate(e, at(x = 1, y = 2));
        static_assert(result[0, 0] == 1);
        static_assert(result[0, 1] == 2);
        static_assert(result[1, 0] == 2);
        static_assert(result[1, 1] == 2);
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
        constexpr auto expr = t*v;
        constexpr auto result = evaluate(expr, at(x = 1, y = 2));
        static_assert(expr.shape == adpp::md_shape<2, 1>{});
        static_assert(result[0] == 1*1 + 1*2*2);
        static_assert(result[1] == 1*2*1 + 2*2);
    };

    "tensor_expression_tensor_dot"_test = [] () {
        var x;
        var y;
        constexpr tensor_expression t0 = {adpp::md_shape<2, 2>{}, x, x*y, y*x, y};
        constexpr tensor_expression t1 = {adpp::md_shape<2, 2>{}, x, x+y, y*y, y};
        constexpr auto expr = t0*t1;
        constexpr auto result = evaluate(expr, at(x = 1, y = 2));
        static_assert(expr.shape == adpp::md_shape<2, 2>{});
        static_assert(result[0, 0] == 1*1 + 1*2*(2*2));
        static_assert(result[0, 1] == 1*(1 + 2) + 1*2*2);
        static_assert(result[1, 0] == (1*2)*1 + 2*2*2);
        static_assert(result[1, 1] == (2*1)*(1+2) + 2*2);
    };

    "md_tensor_expression_tensor_dot"_test = [] () {
        tensor t0{shape<1, 2, 3>};
        tensor t1{shape<3, 2>};
        constexpr auto expr = t0*t1;
        static_assert(expr.shape == adpp::md_shape<1, 2, 2>{});
        constexpr auto result = evaluate(expr, at(t0 = {1, 2, 3, 4, 5, 6}, t1 = {1, 2, 3, 4, 5, 6}));
        static_assert(result[0, 0, 0] == 1*1 + 2*3 + 3*5);
        static_assert(result[0, 0, 1] == 1*2 + 2*4 + 3*6);
        static_assert(result[0, 1, 0] == 4*1 + 5*3 + 6*5);
        static_assert(result[0, 1, 1] == 4*2 + 5*4 + 6*6);
    };

    "md_tensor_expression_tensor_dot_same_size"_test = [] () {
        tensor t0{shape<2, 2>};
        tensor t1{shape<2, 2>};
        constexpr auto expr = t0*t1;
        static_assert(expr.shape == adpp::md_shape<2, 2>{});
        constexpr auto result = evaluate(expr, at(t0 = {1, 2, 3, 4}, t1 = {1, 2, 3, 4}));
        static_assert(result[0, 0] == 1*1 + 2*3);
        static_assert(result[0, 1] == 1*2 + 2*4);
        static_assert(result[1, 0] == 3*1 + 4*3);
        static_assert(result[1, 1] == 3*2 + 4*4);
    };

    "vector_expression_jacobian"_test = [] () {
        using namespace adpp::indices;

        var x;
        var y;
        constexpr vector_expression expr{x + y, x*y, x + cval<2>*y};
        constexpr auto jac = expr.jacobian(at(x = 1, y = 2));

        static_assert(jac[_0, x] == 1);
        static_assert(jac[_0, y] == 1);

        static_assert(jac[_1, x] == 2);
        static_assert(jac[_1, y] == 1);

        static_assert(jac[_2, x] == 1);
        static_assert(jac[_2, y] == 2);
    };

    "md_newton"_test = [] () {
        using namespace adpp::indices;
        vec<2> v;
        const auto& a = v[_0];
        const auto& b = v[_1];
        vector_expression pde{cval<0.5>*a*a + b, cval<5>*a*b + cval<2>}; // TODO: Restriction necessary that all equal?

        const auto iterate = [&] (auto& x, const auto& res) {
            const auto jac = pde.jacobian(at(v = x));
            const auto det = 1.0/(jac[_0, a]*jac[_1, b] - jac[_0, b]*jac[_1, a]);
            x[0] += -( jac[_1, b]*res[0] - jac[_0, b]*res[1])*det;
            x[1] += -(-jac[_1, a]*res[0] + jac[_0, a]*res[1])*det;
        };

        const auto solve = [&] () {
            int i = 0;
            std::array x{5.0, 1.0};
            auto res = pde.evaluate(at(v = x));
            while ((res[0]*res[0] > 1e-12 || res[1]*res[1] > 1e-12) && i < 20) {
                iterate(x, res);
                res = pde.evaluate(at(v = x));
                i++;
            }
            return std::make_pair(i, res);
        };

        const auto [it, res] = solve();
        std::cout << "residual (it = " << it << ") = " << res[0] << ", " << res[1] << std::endl;
        expect(le(res[0], 1e-6));
        expect(le(res[1], 1e-6));
    };

    return EXIT_SUCCESS;
}
