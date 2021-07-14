#ifndef _HUAWEI_VECTORJITTRANSFORM_HPP
#define _HUAWEI_VECTORJITTRANSFORM_HPP

#include <algorithm>
#include <vector>

#include "PtrIteratorWrap.hpp"
#include "functors.hpp"

/**
 * @brief Apply a function to a random access range online, without modifying
 * the underlying values.
 *
 * @tparam IterT
 * @tparam UnaryOp
 */
template <typename IterT, template <typename T> class OpTmpl = identity_functor>
class ContainerJitMap {
    using T = typename std::iterator_traits<IterT>::value_type;
    using UnaryOp = OpTmpl<T>;

    template <typename Iter>
    class custom_const_iterator : public Iter {

    public:
        using iterator_category = typename std::iterator_traits<Iter>::iterator_category;
        using difference_type = typename std::iterator_traits<Iter>::difference_type;
        using value_type = typename std::iterator_traits<Iter>::value_type;
        using pointer = typename std::iterator_traits<Iter>::pointer;
        using reference = typename std::iterator_traits<Iter>::reference;

        custom_const_iterator(Iter it) : Iter(it){};
        inline auto operator*() { return OpTmpl<value_type>()(Iter::operator*()); }
    };

    template <typename T>
    class custom_const_iterator<T*> : public custom_const_iterator<PtrIteratorWrap<T>> {
    public:
        custom_const_iterator(T* ptr) : custom_const_iterator<PtrIteratorWrap<T>>(PtrIteratorWrap(ptr)) { }
    };

public:
    ContainerJitMap(IterT _first, IterT _last) : first(_first), last(_last){};

    inline auto operator[](size_t index) const { return UnaryOp()(first[index]); }
    inline auto at(size_t index) const { return UnaryOp()(first[index]); }
    inline auto begin() const { return custom_const_iterator<IterT>(first); }
    inline auto end() const { return custom_const_iterator<IterT>(last); }
    inline auto size() const { return std::distance(first, last); }

private:
    IterT first;
    IterT last;
};

#endif