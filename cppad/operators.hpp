#pragma once

#include <cmath>

#include <cppad/concepts.hpp>
#include <cppad/expression.hpp>
#include <cppad/detail.hpp>


namespace cppad {

#ifndef DOXYGEN
namespace detail {

    template<concepts::Expression E, typename Mapping>
    void export_sub_expression_to(std::ostream& out, E&& e, const Mapping& mapping, bool add_braces = true) {
        if constexpr (traits::IsVariable<std::remove_cvref_t<E>>::value) {
            out << mapping.name_of(e);
        } else {
            if (add_braces) out << "(";
            e.export_to(out, mapping);
            if (add_braces) out << ")";
        }
    }

    template<concepts::Expression E1, concepts::Expression E2, typename Mapping>
    void export_binary_expression_to(std::ostream& out, E1&& e1, E2&& e2, const char* op_symbol, const Mapping& mapping) {
        export_sub_expression_to(out, std::forward<E1>(e1), mapping);
        out << op_symbol;
        export_sub_expression_to(out, std::forward<E2>(e2), mapping);
    }

    template<concepts::Expression A, concepts::Expression B>
    class BinaryOperationBase {
     public:
        constexpr explicit BinaryOperationBase(A a, B b) noexcept
        : _a{std::forward<A>(a)}
        , _b{std::forward<B>(b)}
        {}

     protected:
        constexpr decltype(auto) get_a() const noexcept { return _a.get(); }
        constexpr decltype(auto) get_b() const noexcept { return _b.get(); }

     private:
        detail::Storage<A> _a;
        detail::Storage<B> _b;
    };

}  // namespace detail
#endif  // DOXYGEN


template<typename... Args>
    requires(std::conjunction_v<traits::IsNamedVariable<Args>...>)
class VariableNameMapping {
    template<typename T> struct NotLValue : std::bool_constant<!std::is_lvalue_reference_v<T>> {};
    static_assert(std::conjunction_v<NotLValue<Args>...>);
 public:
    constexpr VariableNameMapping(Args&&... maps)
    : _map{std::forward<Args>(maps)...}
    {}

    template<typename T> requires(traits::IsVariable<std::remove_cvref_t<T>>::value)
    const char* name_of(const T& var) const {
        return std::apply([&] <typename... NV> (const NV&... named_vars) {
            return _name_of(var, named_vars...);
        }, _map);
    }

 private:
    template<typename Var>
    const char* _name_of(const Var& var) const {}

    template<typename Var, typename Map, typename... Maps>
    const char* _name_of(const Var& var, Map&& map0, Maps&&... maps) const {
        if (map0 == var)
            return map0.name();
        if constexpr (sizeof...(maps) > 0)
            return _name_of(var, std::forward<Maps>(maps)...);
        else
            return "UNDEFINED";
    }

    std::tuple<std::remove_cvref_t<Args>...> _map;
};


template<concepts::Expression A, concepts::Expression B>
class Plus : public ExpressionBase, detail::BinaryOperationBase<A, B> {
 public:
    constexpr explicit Plus(A a, B b) noexcept
    : detail::BinaryOperationBase<A, B>(
        std::forward<A>(a),
        std::forward<B>(b))
    {}

    template<typename NameMapping>
    void export_to(std::ostream& out, const NameMapping& mapping) const {
        detail::export_binary_expression_to(out, this->get_a(), this->get_b(), " + ", mapping);
    }

    constexpr auto value() const noexcept {
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
    constexpr explicit Times(A a, B b) noexcept
    : detail::BinaryOperationBase<A, B>(
        std::forward<A>(a),
        std::forward<B>(b))
    {}

    template<typename NameMapping>
    void export_to(std::ostream& out, const NameMapping& mapping) const {
        detail::export_binary_expression_to(out, this->get_a(), this->get_b(), "*", mapping);
    }

    constexpr auto value() const noexcept {
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
    constexpr explicit Exponential(E e) noexcept
    : _storage(std::forward<E>(e))
    {}

    template<typename NameMapping>
    void export_to(std::ostream& out, const NameMapping& mapping) const {
        out << "exp(";
        detail::export_sub_expression_to(out, _storage.get(), mapping, false);
        out << ")";
    }

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
