#pragma once

#include <cmath>

#include <cppad/concepts.hpp>
#include <cppad/expression.hpp>
#include <cppad/detail.hpp>


namespace cppad {

#ifndef DOXYGEN
namespace detail {

    template<concepts::Expression A, concepts::Expression B>
    class BinaryOperationBase {
     public:
        constexpr explicit BinaryOperationBase(A a, B b)
        : _a{std::forward<A>(a)}
        , _b{std::forward<B>(b)}
        {}

     protected:
        constexpr decltype(auto) get_a() const {
            return _a.get();
        }

        constexpr decltype(auto) get_b() const {
            return _b.get();
        }

     private:
        detail::Storage<A> _a;
        detail::Storage<B> _b;
    };

}  // namespace detail
#endif  // DOXYGEN


template<concepts::Expression A, concepts::Expression B>
class Plus : public ExpressionBase, detail::BinaryOperationBase<A, B> {
 public:
    constexpr explicit Plus(A a, B b)
    : detail::BinaryOperationBase<A, B>(
        std::forward<A>(a),
        std::forward<B>(b))
    {}

    constexpr auto value() const {
        return this->get_a().value() + this->get_b().value();
    }

    template<concepts::Expression E>
    constexpr double partial(E&& e) const {
        return this->partial_to(std::forward<E>(e), [&] (auto&& e) {
            return this->get_a().partial(e) + this->get_b().partial(e);
        });
    }
};

template<concepts::Expression A, concepts::Expression B>
class Times : public ExpressionBase, detail::BinaryOperationBase<A, B> {
 public:
    constexpr explicit Times(A a, B b)
    : detail::BinaryOperationBase<A, B>(
        std::forward<A>(a),
        std::forward<B>(b))
    {}

    constexpr auto value() const {
        return this->get_a().value() * this->get_b().value();
    }

    template<concepts::Expression E>
    constexpr double partial(E&& e) const {
        return this->partial_to(std::forward<E>(e), [&] (auto&& e) {
            return this->get_a().partial(e)*this->get_b().value()
                + this->get_a().value()*this->get_b().partial(e);
        });
    }
};

template<concepts::Expression E>
class Exponential : public ExpressionBase {
 public:
    constexpr explicit Exponential(E e)
    : _storage(std::forward<E>(e))
    {}

    constexpr auto value() const {
        using std::exp;
        return exp(_storage.get().value());
    }

    template<concepts::Expression _E>
    constexpr double partial(_E&& e) const {
        return this->partial_to(std::forward<_E>(e), [&] (auto&& e) {
            return value()*_storage.get().partial(e);
        });
    }

 private:
    detail::Storage<E> _storage;
};

}  // namespace cppad::concepts
