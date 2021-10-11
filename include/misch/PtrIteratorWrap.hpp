#ifndef CAV_PTRITERATORWRAP_HPP
#define CAV_PTRITERATORWRAP_HPP

#include <iterator>

namespace cav {
    /**
     * @brief Iterator class for raw pointers
     *
     * @tparam T type poited by the pointer
     */
    template <class T>
    class PtrIteratorWrap {
    public:
        using iterator_category = typename std::iterator_traits<T*>::iterator_category;
        using difference_type = typename std::iterator_traits<T*>::difference_type;
        using value_type = typename std::iterator_traits<T*>::value_type;
        using pointer = typename std::iterator_traits<T*>::pointer;
        using reference = typename std::iterator_traits<T*>::reference;

        PtrIteratorWrap(T* _current) : current(_current){};

        inline PtrIteratorWrap operator++() noexcept { return ++current, *this; }
        inline PtrIteratorWrap operator++(int) noexcept { return PtrIteratorWrap(current++); }
        inline PtrIteratorWrap operator+(difference_type x) const noexcept { return PtrIteratorWrap(current + x); }
        inline PtrIteratorWrap operator+=(difference_type i) noexcept { return current += i, *this; }

        inline PtrIteratorWrap operator--() noexcept { return --current, *this; }
        inline PtrIteratorWrap operator--(int) noexcept { return PtrIteratorWrap(current--); }
        inline PtrIteratorWrap operator-(difference_type x) const noexcept { return PtrIteratorWrap(current - x); }
        inline PtrIteratorWrap operator-=(difference_type i) noexcept { return current -= i, *this; }
        inline difference_type operator-(PtrIteratorWrap x) const noexcept { return current - x.current; }

        inline bool operator!=(PtrIteratorWrap x) const noexcept { return x.current != current; }
        inline bool operator==(PtrIteratorWrap x) const noexcept { return x.current == current; }
        inline bool operator>(PtrIteratorWrap x) const noexcept { return x.current > current; }
        inline bool operator<(PtrIteratorWrap x) const noexcept { return x.current < current; }
        inline bool operator>=(PtrIteratorWrap x) const noexcept { return x.current >= current; }
        inline bool operator<=(PtrIteratorWrap x) const noexcept { return x.current <= current; }

        inline pointer operator->() const noexcept { return current; }
        inline pointer base() const noexcept { return current; }
        inline reference operator*() const noexcept { return *current; }
        inline reference operator[](difference_type x) const noexcept { return current[x]; }

    private:
        pointer current;
    };
}  // namespace cav

#endif