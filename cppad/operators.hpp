#pragma once

#include <cppad/concepts.hpp>
#include <cppad/expression.hpp>


namespace cppad {

#ifndef DOXYGEN
namespace detail {

    template<typename E>
    class ExpressionStorage {
        using Stored = std::conditional_t<
            std::is_lvalue_reference_v<E>,
            std::add_const_t<E>,
            std::remove_cvref_t<E>
        >;

     public:
        template<std::convertible_to<Stored> _E>
        ExpressionStorage(_E&& e)
        : _expression{std::forward<_E>(e)}
        {}

        decltype(auto) get() const {
            return _expression;
        }

     private:
        Stored _expression;
    };

    template<concepts::Expression A, concepts::Expression B>
    class BinaryOperationBase {
     public:
        explicit BinaryOperationBase(A a, B b)
        : _a{std::forward<A>(a)}
        , _b{std::forward<B>(b)}
        {}

     protected:
        decltype(auto) get_a() const {
            return _a.get();
        }

        decltype(auto) get_b() const {
            return _b.get();
        }

     private:
        ExpressionStorage<A> _a;
        ExpressionStorage<B> _b;
    };

}  // namespace detail
#endif  // DOXYGEN


template<concepts::Expression A, concepts::Expression B>
class Plus : ExpressionBase, detail::BinaryOperationBase<A, B> {
 public:
    explicit Plus(A a, B b)
    : detail::BinaryOperationBase<A, B>(
        std::forward<A>(a),
        std::forward<B>(b))
    {}

    auto value() const {
        return this->get_a().value() + this->get_b().value();
    }

    template<concepts::Expression E>
    double partial(E&& e) const {
        return this->partial_to(std::forward<E>(e), [&] (auto&& e) {
            return this->get_a().partial(e) + this->get_b().partial(e);
        });
    }
};

template<concepts::Expression A, concepts::Expression B>
class Times : ExpressionBase, detail::BinaryOperationBase<A, B> {
 public:
    explicit Times(A a, B b)
    : detail::BinaryOperationBase<A, B>(
        std::forward<A>(a),
        std::forward<B>(b))
    {}

    auto value() const {
        return this->get_a().value() * this->get_b().value();
    }

    template<concepts::Expression E>
    double partial(E&& e) const {
        return this->partial_to(std::forward<E>(e), [&] (auto&& e) {
            return this->get_a().partial(e)*this->get_b().value()
                + this->get_a().value()*this->get_b().partial(e);
        });
    }
};

}  // namespace cppad::concepts
