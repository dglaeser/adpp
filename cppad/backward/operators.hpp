#pragma once

#include <cmath>
#include <ostream>
#include <utility>

#include <cppad/common.hpp>
#include <cppad/backward/expression.hpp>


namespace cppad::backward {

#ifndef DOXYGEN
namespace op_detail {

    template<concepts::expression E>
    void export_sub_expression_to(std::ostream& out, E&& e, const auto& mapping, bool add_braces = true) {
        if constexpr (is_variable_v<E>) {
            out << mapping.name_of(e);
        } else {
            if (add_braces) out << "(";
            e.export_to(out, mapping);
            if (add_braces) out << ")";
        }
    }

    template<concepts::expression E1, concepts::expression E2>
    void export_binary_expression_to(std::ostream& out, E1&& e1, E2&& e2, const char* op_symbol, const auto& mapping) {
        export_sub_expression_to(out, std::forward<E1>(e1), mapping);
        out << op_symbol;
        export_sub_expression_to(out, std::forward<E2>(e2), mapping);
    }

    template<concepts::expression A, concepts::expression B>
    class binary_op_base {
     public:
        constexpr explicit binary_op_base(A a, B b) noexcept
        : _a{std::forward<A>(a)}
        , _b{std::forward<B>(b)}
        {}

     protected:
        constexpr decltype(auto) get_a() const noexcept { return _a.get(); }
        constexpr decltype(auto) get_b() const noexcept { return _b.get(); }

     private:
        storage<A> _a;
        storage<B> _b;
    };

}  // namespace op_detail
#endif  // DOXYGEN


template<typename... Ts>
    requires(
        std::conjunction_v<traits::is_named_variable<Ts>...> and
        std::conjunction_v<is_ownable<Ts>...>
    )
class var_name_map {
 public:
    constexpr var_name_map(Ts&&... maps)
    : _map{std::forward<Ts>(maps)...}
    {}

    template<typename T> requires(is_variable_v<T>)
    const char* name_of(const T& var) const {
        return std::apply([&] (const auto&... named_vars) {
            return _name_of(var, named_vars...);
        }, _map);
    }

 private:
    const char* _name_of(const auto& var) const {}

    template<typename... Maps>
    const char* _name_of(const auto& var, auto&& map0, Maps&&... maps) const {
        if (map0 == var)
            return map0.name();
        if constexpr (sizeof...(maps) > 0)
            return _name_of(var, std::forward<Maps>(maps)...);
        else
            return "$not_defined$";
    }

    std::tuple<std::remove_cvref_t<Ts>...> _map;
};


template<typename... Ts>
inline constexpr auto naming(Ts&&... ts) {
    return var_name_map{std::forward<Ts>(ts)...};
}


template<concepts::expression A, concepts::expression B>
class plus : public expression_base, op_detail::binary_op_base<A, B> {
 public:
    constexpr explicit plus(A a, B b) noexcept
    : op_detail::binary_op_base<A, B>(
        std::forward<A>(a),
        std::forward<B>(b))
    {}

    void export_to(std::ostream& out, const auto& mapping) const {
        op_detail::export_binary_expression_to(out, this->get_a(), this->get_b(), " + ", mapping);
    }

    constexpr auto value() const noexcept {
        return this->get_a().value() + this->get_b().value();
    }

    template<concepts::expression E>
    constexpr double partial(E&& e) const {
        return this->partial_to(std::forward<E>(e), [&] (auto&& e) {
            return this->get_a().partial(e) + this->get_b().partial(e);
        });
    }
};


template<concepts::expression A, concepts::expression B>
class minus : public expression_base, op_detail::binary_op_base<A, B> {
 public:
    constexpr explicit minus(A a, B b) noexcept
    : op_detail::binary_op_base<A, B>(
        std::forward<A>(a),
        std::forward<B>(b))
    {}

    void export_to(std::ostream& out, const auto& mapping) const {
        op_detail::export_binary_expression_to(out, this->get_a(), this->get_b(), " - ", mapping);
    }

    constexpr auto value() const noexcept {
        return this->get_a().value() - this->get_b().value();
    }

    template<concepts::expression E>
    constexpr double partial(E&& e) const {
        return this->partial_to(std::forward<E>(e), [&] (auto&& e) {
            return this->get_a().partial(e) - this->get_b().partial(e);
        });
    }
};


template<concepts::expression A, concepts::expression B>
class times : public expression_base, op_detail::binary_op_base<A, B> {
 public:
    constexpr explicit times(A a, B b) noexcept
    : op_detail::binary_op_base<A, B>(
        std::forward<A>(a),
        std::forward<B>(b))
    {}

    void export_to(std::ostream& out, const auto& mapping) const {
        op_detail::export_binary_expression_to(out, this->get_a(), this->get_b(), "*", mapping);
    }

    constexpr auto value() const noexcept {
        return this->get_a().value() * this->get_b().value();
    }

    template<concepts::expression E>
    constexpr double partial(E&& e) const {
        return this->partial_to(std::forward<E>(e), [&] (auto&& e) {
            return this->get_a().partial(e)*this->get_b().value()
                + this->get_a().value()*this->get_b().partial(e);
        });
    }
};

template<concepts::expression E>
class exponential : public expression_base {
 public:
    constexpr explicit exponential(E e) noexcept
    : _storage(std::forward<E>(e))
    {}

    void export_to(std::ostream& out, const auto& mapping) const {
        out << "exp(";
        op_detail::export_sub_expression_to(out, _storage.get(), mapping, false);
        out << ")";
    }

    constexpr auto value() const {
        using std::exp;
        return exp(_storage.get().value());
    }

    template<concepts::expression _E>
    constexpr double partial(_E&& e) const {
        return this->partial_to(std::forward<_E>(e), [&] (auto&& e) {
            return value()*_storage.get().partial(e);
        });
    }

 private:
    storage<E> _storage;
};

}  // namespace cppad::backward
