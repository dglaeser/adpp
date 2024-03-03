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

    "index_reduce"_test = [] () {
        std::array vec{0, 1, 2, 3, 4};
        {
            int sum = adpp::recursive_reduce(adpp::index_range<0, 1>{}, [&] <auto i> (adpp::index_constant<i>, int current) {
                return current + i;
            }, int{0});
            expect(eq(sum, 0));
        }
        {
            int sum = adpp::recursive_reduce(adpp::index_range<0, 3>{}, [&] <auto i> (adpp::index_constant<i>, int current) {
                return current + i;
            }, int{0});
            expect(eq(sum, 3));
        }
        {
            int sum = adpp::recursive_reduce(adpp::index_range<3, 5>{}, [&] <auto i> (adpp::index_constant<i>, int current) {
                return current + i;
            }, int{0});
            expect(eq(sum, 7));
        }
        {
            int sum = adpp::recursive_reduce(adpp::index_range<0, vec.size()>{}, [&] <auto i> (adpp::index_constant<i>, int current) {
                return current + i;
            }, int{0});
            expect(eq(sum, 10));
        }
    };

    return EXIT_SUCCESS;
}
