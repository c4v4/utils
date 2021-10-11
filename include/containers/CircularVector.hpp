/**
 * Circular vector that tries to mimic the structure of stl containers, alowing the use of custom allocators.
 *
 * NOTE: All the following std::uninitialized_stuff are ok with allocators that simply place stuff into preallocated memory with new(memory).
 * In case of a more exotic custom allocator, these parts need to be updated to use the construct/destroy of the allocator.
 *
 */

#ifndef CAV_CIRCULARVECTOR_HPP
#define CAV_CIRCULARVECTOR_HPP

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>

namespace cav {

    template <typename Int>
    static inline Int next_pow_2(Int v) {
        static_assert(std::is_integral_v<Int>);
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;

        if constexpr (sizeof(Int) < 2) {
            return ++v;
        } else {
            v |= v >> 8;
            if constexpr (sizeof(Int) < 4) {
                return ++v;
            } else {
                v |= v >> 16;
                if constexpr (sizeof(Int) < 8) {
                    return ++v;
                } else {
                    v |= v >> 32;
                    return ++v;
                }
            }
        }
    }

    template <typename T>
    class ArrayProxy {
    public:
        ArrayProxy(T* begin_, T* end_) : b(begin_), e(end_) { }

        T* begin() { return b; }

        T* end() { return e; }

        size_t size() const { return e - b; }

        T& operator[](size_t i) { return b[i]; }

    private:
        T* b;
        T* e;
    };

    template <typename T, typename Alloc = std::allocator<T>>
    class CircularVector : private Alloc {
    public:
        static constexpr size_t INITIAL_CAPACITY = 16UL;

        class circular_iterator {
        public:
            circular_iterator(T* v_, size_t t_, size_t cm1_) : v(v_), p(t_), cm1(cm1_) { }

            inline auto& operator*() { return v[p]; }

            inline auto& operator++() {
                p = (p + 1) & cm1;
                return *this;
            }

            inline auto operator++(int) {
                const auto old_p = p;
                p = (p + 1) & cm1;
                return circular_iterator(v, old_p, cm1);
            }

            inline auto& operator--() {
                p = (p + cm1) & cm1;
                return *this;
            }

            inline auto operator--(int) {
                const auto old_p = p;
                p = (p + cm1) & cm1;
                return circular_iterator(v, old_p, cm1);
            }

            inline bool operator!=(circular_iterator& it2) const { return p != it2.p; }

            inline bool operator==(circular_iterator& it2) const { return p == it2.p; }

        private:
            T* const v;
            size_t p;
            const size_t cm1;
        };

        explicit CircularVector(const CircularVector& cv) : Alloc(cv), h(cv.h), t(cv.t), cm1(cv.cm1), v(Alloc::allocate(cm1 + 1)) {
            std::uninitialized_copy(cv.v, cv.v + cv.cm1 + 1, v);
        }

        explicit CircularVector(CircularVector&& cv) noexcept
            : Alloc(cv), h(std::exchange(cv.h, 0)), t(std::exchange(cv.t, 0)), cm1(std::exchange(cv.cm1, 0)), v(std::exchange(cv.v, nullptr)) { }

        explicit CircularVector(const Alloc& allocator_ = std::allocator<T>())
            : Alloc(allocator_), h(0), t(0), cm1(INITIAL_CAPACITY - 1), v(Alloc::allocate(cm1 + 1)) { }

        CircularVector(size_t size, const Alloc& allocator_ = std::allocator<T>())
            : Alloc(allocator_), h(size), t(0), cm1(std::max(next_pow_2(size + 1), INITIAL_CAPACITY) - 1) {

            v = Alloc::allocate(cm1 + 1);
            std::uninitialized_default_construct(v, v + cm1 + 1);
        }

        CircularVector(size_t size, const T& value, const Alloc& allocator_ = std::allocator<T>())
            : Alloc(allocator_), h(size), t(0), cm1(std::max(next_pow_2(size + 1), INITIAL_CAPACITY) - 1) {

            v = Alloc::allocate(cm1 + 1);
            std::uninitialized_fill(v, v + cm1 + 1, value);
        }

        ~CircularVector() {
            if (t != h) {
                std::destroy(first_half().begin(), first_half().end());
                std::destroy(second_half().begin(), second_half().end());
            }
            Alloc::deallocate(v, cm1 + !!cm1);
        }

        CircularVector operator=(const CircularVector& cv) = delete;

        CircularVector operator=(CircularVector&& cv) = delete;

        static size_t max_size() { return Alloc::max_size(); }

        inline void clear() {
            h = 0;
            t = 0;
        }

        inline void push_back(const T& x) {
            if (full()) { double_capacity(); }
            std::allocator_traits<Alloc>::construct(*this, v + h, x);
            h = (h + 1) & cm1;
        }

        inline void push_back(T&& x) {
            if (full()) { double_capacity(); }
            std::allocator_traits<Alloc>::construct(*this, v + h, std::move(x));
            h = (h + 1) & cm1;
        }

        T& emplace_back(const T& x) {
            push_back(x);
            return back();
        }

        T& emplace_back(T&& x) {
            push_back(std::move(x));
            return back();
        }

        inline const T pop_front() {
            assert(!empty());
            const auto old_t = t;
            t = (t + 1) & cm1;
            return std::move(v[old_t]);
        }

        inline void double_capacity() { _reserve((cm1 + 1) * 2); }

        inline void reserve(size_t size) {
            if (size > cm1 + 1) { _reserve(next_pow_2(size)); }
        }

        inline bool empty() const { return h == t; }

        inline bool full() const { return ((h + 1) & cm1) == t; }

        inline size_t size() const { return ((cm1 + 1) + h - t) & cm1; }

        inline T& back() { return v[(h + cm1) & cm1]; }
        inline const T& back() const { return v[(h + cm1) & cm1]; }

        inline T& operator[](size_t n) { return v[(t + n) & cm1]; }
        inline const T& operator[](size_t n) const { return v[(t + n) & cm1]; }

        inline auto begin() { return circular_iterator(v, t, cm1); }

        inline auto end() { return circular_iterator(v, h, cm1); }

        inline auto first_half() { return ArrayProxy<T>(v + t, v + (t < h ? h : cm1 + 1)); }

        inline auto second_half() { return ArrayProxy<T>(v + static_cast<size_t>(t < h) * h, v + h); }

        void print() {
            for (const auto i : *this) { std::cout << i << " "; }
            std::cout << "\n";
        }

    private:
        inline void _reserve(size_t size) {
            size_t cm1_new = size - 1;
            T* v_new = Alloc::allocate(size);

            size_t size1 = first_half().size();
            size_t h_new = size1 + second_half().size();

            std::uninitialized_move(first_half().begin(), first_half().end(), v_new);
            std::uninitialized_move(second_half().begin(), second_half().end(), v_new + size1);

            Alloc::deallocate(v, cm1 + 1);
            t = 0UL;
            v = v_new;
            h = h_new;
            cm1 = cm1_new;
        }

    private:
        size_t h;
        size_t t;
        size_t cm1;
        T* v;
    };

}  // namespace cav

#endif