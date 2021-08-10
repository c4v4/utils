#ifndef _HUAWEI_INT_PICKER_HPP_
#define _HUAWEI_INT_PICKER_HPP_

#include <cstddef>
#include <cstdint>

// SIGNED
template <int T>
struct int_picker;

template <>
struct int_picker<1> {
    typedef int8_t int_t;
    static constexpr size_t size = 8u;
};

template <>
struct int_picker<2> {
    typedef int16_t int_t;
    static constexpr size_t size = 16u;
};

template <>
struct int_picker<4> {
    typedef int32_t int_t;
    static constexpr size_t size = 32u;
};

template <>
struct int_picker<8> {
    typedef int64_t int_t;
    static constexpr size_t size = 64u;
};

template <>
struct int_picker<16> {
    typedef __int128_t int_t;
    static constexpr size_t size = 128u;
};

// UNSIGNED
template <int T>
struct uint_picker;

template <>
struct uint_picker<1> {
    typedef uint8_t uint_t;
    static constexpr size_t size = 8u;
};

template <>
struct uint_picker<2> {
    typedef uint16_t uint_t;
    static constexpr size_t size = 16u;
};

template <>
struct uint_picker<4> {
    typedef uint32_t uint_t;
    static constexpr size_t size = 32u;
};

template <>
struct uint_picker<8> {
    typedef uint64_t uint_t;
    static constexpr size_t size = 64u;
};

template <>
struct uint_picker<16> {
    typedef __uint128_t uint_t;
    static constexpr size_t size = 128u;
};

#endif