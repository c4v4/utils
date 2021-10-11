#ifndef CAV_FUNCTORS_HPP
#define CAV_FUNCTORS_HPP

#include <type_traits>

namespace cav {
    template <typename T>
    struct identity_ftor {
        constexpr T&& operator()(T&& t) const noexcept { return std::forward<T>(t); }
    };

    template <typename Struct, typename fieldType, fieldType Struct::*field>
    struct base_get_field_ref {
        auto& operator()(Struct& t) const { return t.*field; }
    };

    template <typename Struct, auto field>
    struct get_field_type {
        typedef typename std::decay<decltype(std::declval<Struct>().*field)>::type type;
    };

    template <typename Struct, auto field>
    struct get_field_ref : base_get_field_ref<Struct, typename get_field_type<Struct, field>::type, field> { };

}  // namespace cav
#endif