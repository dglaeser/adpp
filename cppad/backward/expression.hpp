#pragma once

#include <concepts>
#include <utility>
#include <type_traits>
#include <ostream>
#include <cmath>

#include <cppad/common.hpp>


namespace cppad::backward {

// forward declarations
template<concepts::expression A, concepts::expression B> class minus;
template<concepts::expression A, concepts::expression B> class plus;
template<concepts::expression A, concepts::expression B> class times;
template<concepts::expression E> class exponential;

struct expression_base {
    template<typename Self, concepts::into_expression Other>
    constexpr auto operator+(this Self&& self, Other&& other) noexcept {
        using OtherExpression = decltype(as_expression(std::forward<Other>(other)));
        return plus<Self, OtherExpression>{
            std::forward<Self>(self),
            as_expression(std::forward<Other>(other))
        };
    }

    template<typename Self, concepts::into_expression Other>
    constexpr auto operator-(this Self&& self, Other&& other) noexcept {
        using OtherExpression = decltype(as_expression(std::forward<Other>(other)));
        return minus<Self, OtherExpression>{
            std::forward<Self>(self),
            as_expression(std::forward<Other>(other))
        };
    }

    template<typename Self, concepts::into_expression Other>
    constexpr auto operator*(this Self&& self, Other&& other) noexcept {
        using OtherExpression = decltype(as_expression(std::forward<Other>(other)));
        return times<Self, OtherExpression>{
            std::forward<Self>(self),
            as_expression(std::forward<Other>(other))
        };
    }

    template<typename Self>
    constexpr auto exp(this Self&& self) {
        return exponential<Self>{std::forward<Self>(self)};
    }

 protected:
    template<typename Self, concepts::expression E, std::invocable<E> Partial>
        requires(concepts::arithmetic<std::invoke_result_t<Partial, E>>)
    constexpr double partial_to(this Self&& self, E&& e, Partial&& partial) {
        static_assert(
            !traits::is_constant<std::remove_cvref_t<E>>::value,
            "Derivative w.r.t. a constant requested"
        );
        if constexpr (!std::is_same_v<std::remove_cvref_t<Self>, std::remove_cvref_t<E>>)
            return partial(std::forward<E>(e));
        else
            return is_same_object(self, e) ? 1.0 : partial(std::forward<E>(e));
    }
};

template<concepts::expression E>
class named_expression {
    static_assert(std::is_lvalue_reference_v<E>);

 public:
    constexpr named_expression(E e, const char* name) noexcept
    : _storage{std::forward<E>(e)}
    , _name{name}
    {}

    constexpr const char* name() const {
        return _name;
    }

    template<concepts::expression _E>
    constexpr bool operator==(_E&& e) const {
        return is_same_object(e, _storage.get());
    }

 private:
    storage<E> _storage;
    const char* _name;
};


template<concepts::expression E>
class expression {
 public:
    constexpr expression(E&& e)
    : _e{std::move(e)}
    {}

    constexpr decltype(auto) value() const {
        return _e.value();
    }

    template<concepts::expression _E>
    constexpr decltype(auto) partial(_E&& e) const {
        return _e.partial(std::forward<_E>(e));
    }

 private:
    E _e;
};

template<concepts::expression E>
expression(E&&) -> expression<std::remove_cvref_t<E>>;

}  // namespace cppad::backward

namespace cppad::traits {

template<concepts::expression V> requires(is_variable_v<V>)
struct is_variable<cppad::backward::named_expression<V>> : public std::true_type {};

template<concepts::expression V> requires(is_variable_v<V>)
struct is_named_variable<cppad::backward::named_expression<V>> : public std::true_type {};

}  // namespace cppad::traits

namespace std {

template<cppad::concepts::expression E>
constexpr auto exp(E&& e) {
    return std::forward<E>(e).exp();
}

}  // namespace std
