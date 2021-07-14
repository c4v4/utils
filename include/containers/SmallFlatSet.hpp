#ifndef UTILS_INCLUDE_SMALLFLATSET_HPP
#define UTILS_INCLUDE_SMALLFLATSET_HPP

#include <algorithm>

#include "functor.hpp"

static constexpr int next2pow(int v) {
    --v;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return ++v;
}

template <typename Value, Value emptyValue, int maxSize, class op = identity_functor<Value>>
class SmallFlatSet {

    static_assert(maxSize > 0, "Needs a positive value size.");
    static_assert(std::is_integral_v<decltype(maxSize)>, "Needs a integral type for maxSize");
    static_assert(maxSize * 5 / 4 <= (1 << 16), "Choose a smalle maxSize.");
    static_assert(next2pow(maxSize * 5 / 4) * sizeof(Value) <= (1 << 16), "Maximum memory occupation of 65KB.");

public:
    class custom_iterator {
        friend class SmallFlatSet<Value, emptyValue, maxSize, op>;

    private:
        custom_iterator(Value* _base, Value* _end) : base(_base), end(_end) {
            while (base != end && *base == emptyValue) ++base;
        };

    public:
        inline auto& operator*() { return *base; }

        inline auto operator->() { return base; }

        inline auto& operator++() {
            do { ++base; } while (base != end && *base == emptyValue);
            return *this;
        }

        inline auto operator!=(const custom_iterator x) { return x.base != base; }

        inline auto operator==(const custom_iterator x) { return x.base == base; }

    private:
        Value* base;
        const Value* end;
    };

public:
    SmallFlatSet() {
        for (Value& p : buffer) p = emptyValue;
    };

    inline Value& find(const Value v) {
        size_t index = op()(v) & realSizem1;
        Value value = buffer[index];
        while (value != v && value != emptyValue) {
            index = (index + 1) & realSizem1;
            value = buffer[index];
        }
        return buffer[index];
    }


    inline bool insert(const Value v) {
        Value& candidate_place = find(v);
        if (candidate_place != emptyValue) return false;  // element already there

        candidate_place = v;
        return true;
    }

    inline void insert_or_assign(const Value v) { operator[](v); }

    inline Value& operator[](Value v) {
        Value& value = find(v);
        value = v;
        return value;
    }

    inline void clear() {
        for (Value& p : buffer) p = emptyValue;
    }

    inline size_t count(Value v) { return static_cast<size_t>(find(v) != emptyValue); }

    static inline Value get_emptyvalue() { return emptyValue; }

    static inline size_t get_maxsize() { return maxSize; }

    inline auto begin() { return custom_iterator(buffer, buffer + realSize); };

    inline auto end() { return custom_iterator(buffer + realSize, buffer + realSize); };

private:
    // Why 5/4 do you ask? Clearly a well thought value, not at all the nearest
    // small fraction that transform 25 to ~32.
    constexpr static int realSize = next2pow(maxSize * 5 / 4);
    constexpr static int realSizem1 = realSize - 1;

public:
    Value buffer[realSize];
};


template <typename Value, Value emptyValue, int maxSize>
class VerySmallFlatSet {

    static_assert(maxSize > 0, "Needs a positive value size.");
    static_assert(std::is_integral_v<decltype(maxSize)>, "Needs a integral type for maxSize");
    static_assert(maxSize * 5 / 4 <= (1 << 16), "Choose a smalle maxSize.");
    static_assert(next2pow(maxSize * 5 / 4) * sizeof(Value) <= (1 << 16), "Maximum memory occupation of 65KB.");

public:
    VerySmallFlatSet() {
        for (Value& p : buffer) p = emptyValue;
    };

    inline Value& find(const Value v) {
        int index = 0;
        while (buffer[index] != v && buffer[index] != emptyValue) {
            ++index;
            assert(index < realSize - 1);
        }
        return buffer[index];
    }


    inline bool insert(const Value v) {
        Value& candidate_place = find(v);
        if (candidate_place != emptyValue) return false;  // element already there

        candidate_place = v;
        return true;
    }

    inline bool insert_or_assign(const Value v) {
        // if not there insert it, if there assign it.
        Value& candidate_place = find(v);
        candidate_place = v;

        return true;
    }

    inline Value& operator[](Value v) { return find(v) = v; }

    inline void clear() {
        for (Value& p : buffer) p = emptyValue;
    }

    inline size_t count(Value v) { return static_cast<size_t>(find(v) != emptyValue); }

    static inline Value get_emptyvalue() { return emptyValue; }

    static inline size_t get_maxsize() { return maxSize; }

    inline Value* begin() { return buffer; }

    inline Value* end() { return buffer + realSize; }

private:
    // Why 5/4 do you ask? Clearly a well thought value, not at all the nearest
    // small fraction that transform 25 to ~32.
    constexpr static int realSize = next2pow(maxSize * 5 / 4);
    constexpr static int realSizem1 = realSize - 1;

public:
    Value buffer[realSize];
};

#endif
