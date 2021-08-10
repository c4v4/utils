#ifndef _HUAWEI_VECTORJITTRANSFORM_HPP
#define _HUAWEI_VECTORJITTRANSFORM_HPP

#include <algorithm>
#include <vector>

#include "PtrIteratorWrap.hpp"
#include "functors.hpp"

/**
 * @brief Applies a function to a random access range online, without modifying
 * the underlying values.
 *
 * @tparam IterT
 * @tparam UnaryOp
 */
template <typename IterT, template <typename T> class OpTmpl = identity_functor>
class ContainerJitMap {
    typedef typename std::iterator_traits<IterT>::value_type T;
    typedef OpTmpl<T> UnaryOp;

    template <typename Iter>  // template specialization is not needed since only used methods are instantiated
    struct custom_const_iterator : public PtrIteratorWrap<Iter> {
        custom_const_iterator(Iter it) : PtrIteratorWrap<Iter>(it){};
        inline auto operator*() { return UnaryOp()(PtrIteratorWrap<Iter>::operator*()); }
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