#ifndef CAV_TRIVIALHEAP_HPP
#define CAV_TRIVIALHEAP_HPP

#include <array>
#include <cinttypes>

namespace cav {
    template <typename T, std::size_t Nm, typename Comp>
    class TrivialHeap {
    public:
        inline auto size() const { return sz; }

        inline void pop_back() {
            assert(sz > 0);
            --sz;
        }

        inline auto back() const {
            assert(sz > 0);
            return buf[sz - 1];
        }

        inline auto& back() {
            assert(sz > 0);
            return buf[sz - 1];
        }

        inline auto begin() const { return std::addressof(buf[0]); }
        inline auto begin() { return std::addressof(buf[0]); }
        inline auto end() const { return std::addressof(buf[sz]); }
        inline auto end() { return std::addressof(buf[sz]); }

        inline void insert(T elem) {
            assert(sz < Nm);
            auto* ptr = end();
            while (ptr != begin() && Comp()(*(ptr - 1), elem)) {
                *(ptr) = *(ptr - 1);
                --ptr;
            }
            *ptr = elem;
            ++sz;
        }

        inline void clear() { sz = 0U; }

    private:
        T buf[Nm];
        uint32_t sz = 0U;
    };
}  // namespace cav

#endif