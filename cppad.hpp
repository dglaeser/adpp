#pragma once

#include <utility>
#include <concepts>
#include <type_traits>
#include <memory>

namespace cppad {
namespace Concepts {

template<typename T>
concept Arithmetic = std::floating_point<T> or std::integral<T>;

template<typename T>
concept Expression = requires(const T& t) {
    { t.value() } -> Arithmetic;
};

}  // namespace Concepts

namespace Detail {

    template<typename E>
    class ExpressionStorage {
     public:
        using Stored = std::conditional_t<
            std::is_lvalue_reference_v<E>,
            std::add_const_t<E>,
            std::remove_cvref_t<E>
        >;

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

    template<Concepts::Expression A, Concepts::Expression B>
    class BinaryOperation {
     public:
        explicit BinaryOperation(A a, B b)
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

    template<typename A, typename B>
    constexpr bool is_same_object(A&& a, B&& b) {
        if constexpr (std::same_as<std::remove_cvref_t<A>, std::remove_cvref_t<B>>)
            return std::addressof(a) == std::addressof(b);
        return false;
    }

}  // namespace Detail

template<Concepts::Expression A, Concepts::Expression B>
class Plus : Detail::BinaryOperation<A, B> {
 public:
    explicit Plus(A a, B b)
    : Detail::BinaryOperation<A, B>(
        std::forward<A>(a),
        std::forward<B>(b))
    {
        static_assert(std::is_lvalue_reference_v<A>);
    }

    auto value() const {
        return this->get_a().value() + this->get_b().value();
    }

    template<Concepts::Expression E>
    double partial(E&& e) const {
        return Detail::is_same_object(*this, e)
            ? 1.0
            : this->get_a().partial(e) + this->get_b().partial(e);
    }
};

template<Concepts::Expression A, Concepts::Expression B>
class Times : Detail::BinaryOperation<A, B> {
 public:
    explicit Times(A a, B b)
    : Detail::BinaryOperation<A, B>(
        std::forward<A>(a),
        std::forward<B>(b))
    {}

    auto value() const {
        return this->get_a().value() * this->get_b().value();
    }

    template<Concepts::Expression E>
    double partial(E&& e) const {
        return Detail::is_same_object(*this, e)
            ? 1.0
            : this->get_a().partial(e)*this->get_b().value()
            + this->get_a().value()*this->get_b().partial(e);
    }
};

template<Concepts::Arithmetic V>
class Variable {
 public:
    explicit Variable(V&& v)
    : _value{std::move(v)}
    {}

    template<typename Self, Concepts::Expression Other>
    constexpr auto operator+(this Self&& self, Other&& other) {
        return Plus<Self, Other>{
            std::forward<Self>(self),
            std::forward<Other>(other)
        };
    }

    template<typename Self, Concepts::Expression Other>
    constexpr auto operator*(this Self&& self, Other&& other) {
        return Times<Self, Other>{
            std::forward<Self>(self),
            std::forward<Other>(other)
        };
    }

    V value() const {
        return _value;
    }

    template<Concepts::Expression E>
    double partial(E&& e) const {
        return Detail::is_same_object(*this, e) ? 1.0 : 0.0;
    }

 private:
    V _value;
};

template<typename E>
constexpr auto var(E&& e) {
    return Variable{std::forward<E>(e)};
}

}  // namespace cppad
