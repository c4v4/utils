#ifndef _HUAWEI_PTRITERATORWRAP_HPP
#define _HUAWEI_PTRITERATORWRAP_HPP

#include <iterator>

/**
 * @brief Vrapper iterator class for everithing that might work as iterator and need to be class
 *
 * @tparam T type poited by the pointer
 */
template <class ptrT>
class PtrIteratorWrap {
public:
    typedef typename std::iterator_traits<ptrT>::iterator_category iterator_category;
    typedef typename std::iterator_traits<ptrT>::difference_type difference_type;
    typedef typename std::iterator_traits<ptrT>::value_type value_type;
    typedef typename std::iterator_traits<ptrT>::pointer pointer;
    typedef typename std::iterator_traits<ptrT>::reference reference;

    PtrIteratorWrap(ptrT _current) : current(_current){};

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

#endif