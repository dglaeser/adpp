#include <cstdlib>
#include <type_traits>
#include <algorithm>
#include <array>

#include <boost/ut.hpp>
#include <adpp/common.hpp>

int main() {
    using boost::ut::operator""_test;
    using boost::ut::expect;
    using boost::ut::eq;

    "storage_owned"_test = [] () {
        adpp::storage stored{42.0};
        expect(eq(stored.get(), 42.0));
        static_assert(std::is_same_v<
            decltype(stored.get()),
            double&
        >);
    };

    "storage_owned_const"_test = [] () {
        const adpp::storage stored{42.0};
        expect(eq(stored.get(), 42.0));
        static_assert(std::is_same_v<
            decltype(stored.get()),
            const double&
        >);
    };

    "storage_reference"_test = [] () {
        double value = 42.0;
        adpp::storage stored{value};
        expect(eq(stored.get(), 42.0));
        static_assert(std::is_same_v<
            decltype(stored.get()),
            double&
        >);

        value *= 2;
        expect(eq(stored.get(), 84.0));
    };

    "storage_reference_const"_test = [] () {
        double value = 42.0;
        const adpp::storage stored{value};
        expect(eq(stored.get(), 42.0));
        static_assert(std::is_same_v<
            decltype(stored.get()),
            const double&
        >);

        value *= 2;
        expect(eq(stored.get(), 84.0));
    };

    "for_each_n"_test = [] () {
        std::array vec{0, 1, 2, 3, 4};
        std::array cpy = vec;
        std::ranges::fill(cpy, 0.0);

        expect(!std::ranges::equal(vec, cpy));
        adpp::for_each_n<vec.size()>([&] <auto i> (adpp::index_constant<i>) {
            std::cout << "v[" << i << "] = " << vec[i] << std::endl;
            cpy[i] = vec[i];
        });
        expect(std::ranges::equal(vec, cpy));
    };

    return EXIT_SUCCESS;
}
