#ifndef CAV_VECTORVIEW_HPP
#define CAV_VECTORVIEW_HPP

#include <cstddef>
#include <iterator>
#include <vector>

namespace cav {
    
    /**
     * @brief Just a window over a container for ranged for and other things that need begin() and end().
     *
     * @tparam IterT
     */
    template <typename IterT>
    class VectorView {

        static_assert(std::is_same_v<typename std::iterator_traits<IterT>::iterator_category, std::random_access_iterator_tag>,
                      "The iterator type need to be a random_access_iterator or a pointer.");

    public:
        VectorView(IterT _first, IterT _last) : first(_first), last(_last){};

        inline auto begin() { return first; }
        inline auto begin() const { return first; }

        inline auto end() { return last; }
        inline auto end() const { return last; }

        inline auto& operator[](size_t index) { return first[index]; }
        inline auto& operator[](size_t index) const { return first[index]; }

        inline auto& back() { return *(last - 1); }
        inline auto& back() const { return *(last - 1); }

        inline auto size() { return std::distance(first, last); }
        inline auto size() const { return std::distance(first, last); }

    private:
        IterT first;
        IterT last;
    };
}  // namespace cav

#endif