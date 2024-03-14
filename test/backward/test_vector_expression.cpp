#include <cstdlib>
#include <type_traits>

#include <boost/ut.hpp>

#include <adpp/backward/vector.hpp>
#include <adpp/backward/evaluate.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using adpp::backward::var;
using adpp::backward::vec;
using adpp::backward::vector_expression;

int main() {

    "vector_expression"_test = [] () {
        var x;
        var y;
        vector_expression v = {x, y};
        static_assert(std::is_same_v<
            std::remove_cvref_t<decltype(v)>,
            vector_expression<
                std::remove_cvref_t<decltype(x)>,
                std::remove_cvref_t<decltype(y)>
            >
        >);
    };

    "vec"_test = [] () {
        constexpr vec<3> v;
        constexpr auto result = evaluate(v, at(v = std::array{0, 1, 2}));
        static_assert(adpp::backward::is_symbol_v<std::remove_cvref_t<decltype(v)>>);
        static_assert(std::is_same_v<
            std::remove_cvref_t<decltype(result)>,
            std::array<int, 3>
        >);
        static_assert(result[0] == 0);
        static_assert(result[1] == 1);
        static_assert(result[2] == 2);
    };

    return EXIT_SUCCESS;
}
