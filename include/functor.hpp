#ifndef UTILS_INCLUDE_FUNCTOR_HPP
#define UTILS_INCLUDE_FUNCTOR_HPP

#include <type_traits>

template <typename T>
struct identity_functor {
    auto operator()(T x) { return x; }
};

template <typename T>
struct twin_functor {
    static_assert(std::is_integral_v<T>);
    auto operator()(T x) { return x ^ 1; }
};

template <typename T>
struct base_functor {
    static_assert(std::is_integral_v<T>);
    auto operator()(T x) { return x & (~1); }
};

template <typename T, typename fieldType, fieldType T::*field>
struct base_access_field_functor {
    auto& operator()(T& t) { return t.*field; }
};

template <typename T, auto field>
struct access_field_functor : base_access_field_functor<T, typename std::decay<decltype(std::declval<T>().*field)>::type, field> { };

#endif  // COBRA_INCLUDE_COBRA_FUNCTOR_HPP_