#ifndef UTILS_INCLUDE_SMALLFLATMAP_HPP
#define UTILS_INCLUDE_SMALLFLATMAP_HPP

#include <algorithm>

#include "functor.hpp"

template <typename Key, typename Value, Key emptyKey, int maxSize, class op = identity_functor<Key>>
class SmallFlatMap {

    using KVpair = std::pair<Key, Value>;
    using KVpair_ptr = KVpair*;

    static_assert(maxSize > 0, "Needs a positive value size.");
    static_assert(std::is_integral_v<decltype(maxSize)>, "Needs a integral type for maxSize");
    static_assert(maxSize * 5 / 4 <= (1 << 16), "Choose a smaller maxSize.");
    static constexpr int next2pow(int v) {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        return ++v;
    }
    static_assert(next2pow(maxSize * 5 / 4) * sizeof(KVpair) <= (1 << 16), "Maximum memory occupation: 65KB.");

public:
    class custom_iterator {
        friend class SmallFlatMap<Key, Value, emptyKey, maxSize, op>;

    private:
        custom_iterator(KVpair_ptr _base, KVpair_ptr _end) : base(_base), end(_end) {
            while (base != end && base->first == emptyKey) ++base;
        };

    public:
        inline auto& operator*() { return *base; }

        inline auto operator->() { return base; }

        inline auto& operator++() {
            do { ++base; } while (base != end && base->first == emptyKey);
            return *this;
        }

        inline auto operator!=(const custom_iterator x) { return x.base != base; }

        inline auto operator==(const custom_iterator x) { return x.base == base; }

    private:
        KVpair_ptr base;
        const KVpair_ptr end;
    };

public:
    SmallFlatMap() {
        for (auto& p : buffer) p.first = emptyKey;
    };

    /**
     * @brief Search fo an element.
     *
     * @param k         the key of the element to look for
     * @return Value    The reference to the pair {key, value} if the element has
     * been found, The reference to a pair {emptyKey, <undefined>} otherwise.
     */
    inline KVpair& find(const Key k) {
        size_t index = op()(k) & realSizem1;
        Key key = buffer[index].first;
        while (key != k && key != emptyKey) {
            index = (index + 1) & realSizem1;
            key = buffer[index].first;
        }
        return buffer[index];
    }

    /**
     * @brief Tries to insert a new key-value pair into the map.
     *
     * @param k         key
     * @param v         value
     * @return true     the insertion took place
     * @return false    otherwise
     */
    inline bool insert(const Key k, const Value v) {

        KVpair& candidate_place = find(k);
        if (candidate_place.first != emptyKey) return false;  // element already there

        candidate_place = {k, v};
        return true;
    }

    /**
     * @brief Tries to insert a new key-value pair into the map, if already there,
     * modifies its value.
     *
     * @param k         key
     * @param v         value
     */
    inline void insert_or_assign(const Key k, const Value v) { operator[](k) = v; }

    inline Value& operator[](Key k) {
        KVpair& kv = find(k);
        kv.first = k;
        return kv.second;
    }

    inline void clear() {
        for (KVpair& p : buffer) p.first = emptyKey;
    }

    inline size_t count(Key k) { return static_cast<size_t>(find(k).first != emptyKey); }

    inline auto begin() { return custom_iterator(buffer, buffer + realSize); };

    inline auto end() { return custom_iterator(buffer + realSize, buffer + realSize); };

    static inline Key get_emptykey() { return emptyKey; }

    static inline int get_maxsize() { return maxSize; }

private:
    // Why 5/4 do you ask? Clearly a well thought value, not at all the nearest
    // small fraction that transform 25 to ~32.
    constexpr static int realSize = next2pow(maxSize * 5 / 4);
    constexpr static int realSizem1 = realSize - 1;

public:
    std::pair<Key, Value> buffer[realSize];
};

#endif
